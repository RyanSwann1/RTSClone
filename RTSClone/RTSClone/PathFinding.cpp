#include "PathFinding.h"
#include "Globals.h"
#include "Map.h"
#include "Unit.h"
#include "Harvester.h"
#include "ModelManager.h"
#include "AdjacentPositions.h"
#include <limits>
#include <queue>

namespace
{
	void getPathFromVisitedNodes(const glm::ivec2& startingPosition, const glm::ivec2& destinationPositionOnGrid, 
		std::vector<glm::vec3>& pathToPosition, Graph& graph)
	{
		pathToPosition.push_back(Globals::convertToWorldPosition(destinationPositionOnGrid));
		glm::ivec2 position = graph.getPreviousPosition(destinationPositionOnGrid);

		while (position != startingPosition)
		{
			pathToPosition.push_back(Globals::convertToWorldPosition(position));
			position = graph.getPreviousPosition(position);

			assert(pathToPosition.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
		}
	}

	void getPathFromVisitedNodes(const glm::ivec2& startingPosition, const glm::ivec2& destinationPositionOnGrid, const glm::vec3& destinationPosition, 
		std::vector<glm::vec3>& pathToPosition, Graph& graph)
	{
		pathToPosition.push_back(destinationPosition);
		glm::ivec2 positionOnGrid = graph.getPreviousPosition(destinationPositionOnGrid);

		while (positionOnGrid != startingPosition)
		{
			glm::vec3 position = Globals::convertToWorldPosition(positionOnGrid);
			position.x += static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			pathToPosition.push_back(Globals::convertToWorldPosition(positionOnGrid));
			positionOnGrid = graph.getPreviousPosition(positionOnGrid);

			assert(pathToPosition.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
		}
	}
}

PathFinding::PathFinding()
	: m_graph(),
	m_frontier(),
	m_openQueue(static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)),
	m_closedQueue(static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE))
{}



std::vector<glm::vec3> PathFinding::getFormationPositions(const glm::vec3& startingPosition,
	const std::vector<const Unit*> selectedUnits, const Map& map)
{
	//TODO: Sort by closest
	assert(!selectedUnits.empty() && std::find(selectedUnits.cbegin(), selectedUnits.cend(), nullptr) == selectedUnits.cend());
	m_graph.reset(m_frontier);

	std::vector<glm::vec3> unitFormationPositions;
	unitFormationPositions.reserve(selectedUnits.size());
	int selectedUnitIndex = 0;
	m_frontier.push(Globals::convertToGridPosition(startingPosition));

	while (!m_frontier.empty() && unitFormationPositions.size() < selectedUnits.size())
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions = getAllAdjacentPositions(position, map);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid)
			{
				if (!m_graph.isPositionVisited(adjacentPosition.position))
				{
					m_graph.addToGraph(adjacentPosition.position, position);
					m_frontier.push(adjacentPosition.position);

					assert(selectedUnitIndex < selectedUnits.size());
					unitFormationPositions.emplace_back(Globals::convertToWorldPosition(adjacentPosition.position));
					++selectedUnitIndex;
					
					if (unitFormationPositions.size() == selectedUnits.size())
					{
						break;
					}
				}
			}
		}
	}

	return unitFormationPositions;
}

glm::vec3 PathFinding::getClosestAvailablePosition(const glm::vec3& startingPosition, const std::vector<Unit>& units, const Map& map)
{
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(startingPosition);
	if (Globals::isPositionInMapBounds(startingPositionOnGrid) && !map.isPositionOccupied(startingPositionOnGrid))
	{
		auto cIter = std::find_if(units.cbegin(), units.cend(), [&startingPosition](const auto& unit)
		{
			return unit.getPosition() == startingPosition;
		});

		if (cIter == units.cend())
		{
			return startingPosition;
		}
	}

	m_graph.reset(m_frontier);
	m_frontier.push(startingPositionOnGrid);
	glm::ivec2 availablePositionOnGrid = {0, 0};
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions = getAllAdjacentPositions(position, map, units);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid)
			{
				if (!adjacentPosition.unitCollision)
				{
					availablePositionOnGrid = adjacentPosition.position;
					availablePositionFound = true;
					break;
				}
				else if (!m_graph.isPositionVisited(adjacentPosition.position))
				{
					m_graph.addToGraph(adjacentPosition.position, position);
					m_frontier.push(adjacentPosition.position);
				}
			}
		}

		assert(m_frontier.size() < Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	return Globals::convertToWorldPosition(availablePositionOnGrid);
}

glm::vec3 PathFinding::getClosestPositionOutsideAABB(const glm::vec3& entityPosition, const AABB& AABB, const glm::vec3& centrePositionAABB, 
	const Map& map)
{
	glm::vec3 closestPosition = centrePositionAABB;
	glm::vec3 position = centrePositionAABB;
	glm::vec3 direction = glm::normalize(entityPosition - centrePositionAABB);
	for (float ray = 1.0f; ray <= Globals::NODE_SIZE * 5.0f; ++ray)
	{
		position = position + direction * ray;
		if (!AABB.contains(position) && !map.isPositionOccupied(position) && Globals::isPositionInMapBounds(position))
		{
			closestPosition = position;
			break;
		}
	}

	return closestPosition;
}

void PathFinding::getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const GetAllAdjacentPositions& getAdjacentPositions, bool includeWorldDestinationPosition)
{
	assert(getAdjacentPositions);

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);

	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			assert(pathToPosition.empty());
			if (includeWorldDestinationPosition)
			{
				pathToPosition.push_back(destination);
			}

			pathToPosition.emplace_back(Globals::convertToWorldPosition(currentNode.position));

			glm::ivec2 parentPosition = currentNode.parentPosition;
			while (parentPosition != startingPositionOnGrid)
			{
				const PriorityQueueNode& parentNode = m_closedQueue.getNode(parentPosition);
				parentPosition = parentNode.parentPosition;

				glm::vec3 position = Globals::convertToWorldPosition(parentNode.position);
				pathToPosition.emplace_back(position);
			}

			destinationReached = true;
		}
		else
		{
			std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions = getAdjacentPositions(currentNode.position);
			for (const auto& adjacentPosition : adjacentPositions)
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + glm::distance(glm::vec2(adjacentPosition.position), glm::vec2(currentNode.position)),
						glm::distance(glm::vec2(destinationOnGrid), glm::vec2(adjacentPosition.position)));

					if (m_openQueue.isSuccessorNodeValid(adjacentNode))
					{
						m_openQueue.getNode(adjacentPosition.position) = adjacentNode;
					}
					else if (!m_openQueue.contains(adjacentPosition.position))
					{
						m_openQueue.add(adjacentNode);
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(m_openQueue.getSize() <= static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE) &&
			m_closedQueue.getSize() <= static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE));
	}
}

void PathFinding::convertPathToWaypoints(std::vector<glm::vec3>& pathToPosition, const Unit& unit, const std::vector<Unit>& units,
	const Map& map)
{
	assert(!units.empty());
	if (pathToPosition.size() <= size_t(1))
	{
		return;
	}

	int MAX_RAY = (glm::distance(pathToPosition.front(), pathToPosition.back()));
	std::cout << MAX_RAY << "\n";

	glm::vec3 position = unit.getPosition();
	glm::vec3 direction = glm::normalize(pathToPosition.front() - position);
	bool collision = false;
	for (float ray = 1.0f; ray <= MAX_RAY; ++ray)
	{
		position += direction * 1.0f;
		Globals::printImmediately(position);

		auto cIter = std::find_if(units.cbegin(), units.cend(), [&position, &unit](const auto& otherUnit)
		{
			return &unit != &otherUnit && otherUnit.getAABB().contains(position);
		});
		if (cIter != units.cend() || map.isPositionOccupied(position))
		{
			collision = true;
			break;
		}
	}

	if (!collision)
	{
		while (pathToPosition[0] != pathToPosition.back())
		{
			pathToPosition.erase(pathToPosition.begin() + 1);
		}
	}
}