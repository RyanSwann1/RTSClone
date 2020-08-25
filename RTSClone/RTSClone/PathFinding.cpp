#include "PathFinding.h"
#include "Globals.h"
#include "Map.h"
#include "Unit.h"
#include "Worker.h"
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

	void getPathFromClosedQueue(std::vector<glm::vec3>& pathToPosition, const glm::ivec2& startingPositionOnGrid,
		const PriorityQueueNode& initialNode, const PriorityQueue& closedQueue)
	{
		pathToPosition.push_back(Globals::convertToWorldPosition(initialNode.position));
		glm::ivec2 parentPosition = initialNode.parentPosition;

		while (parentPosition != startingPositionOnGrid)
		{
			const PriorityQueueNode& parentNode = closedQueue.getNode(parentPosition);
			parentPosition = parentNode.parentPosition;

			pathToPosition.push_back(Globals::convertToWorldPosition(parentNode.position));
			assert(pathToPosition.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
		}
	}

	void getPathFromClosedQueue(std::vector<glm::vec3>& pathToPosition, const glm::ivec2& startingPositionOnGrid,
		const glm::ivec2& initialPathPosition, const PriorityQueue& closedQueue)
	{
		pathToPosition.push_back(Globals::convertToWorldPosition(initialPathPosition));
		glm::ivec2 parentPosition = closedQueue.getNode(initialPathPosition).parentPosition;

		while (parentPosition != startingPositionOnGrid)
		{
			const PriorityQueueNode& parentNode = closedQueue.getNode(parentPosition);
			parentPosition = parentNode.parentPosition;

			pathToPosition.push_back(Globals::convertToWorldPosition(parentNode.position));
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

bool PathFinding::isPositionAvailable(const glm::vec3& nodePosition, const Map& map, const std::list<Unit>& units, const std::list<Worker>& workers, 
	const Worker& workerSender) const
{
	assert(nodePosition == Globals::convertToNodePosition(nodePosition));

	if (!map.isPositionOccupied(nodePosition))
	{
		auto unit = std::find_if(units.cbegin(), units.cend(), [&nodePosition](const auto& unit) -> bool
		{
			return Globals::convertToNodePosition(unit.getPosition()) == nodePosition;
		});

		if (unit == units.cend())
		{
			auto worker = std::find_if(workers.cbegin(), workers.cend(), [&nodePosition, &workerSender](const auto& worker) -> bool
			{
				return &worker != &workerSender && Globals::convertToNodePosition(worker.getPosition()) == nodePosition;
			});
			if (worker != workers.cend())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	return false;
}

bool PathFinding::isPositionAvailable(const glm::vec3& nodePosition, const Map& map, const std::list<Unit>& units, const std::list<Worker>& workers) const
{
	assert(nodePosition == Globals::convertToNodePosition(nodePosition));

	if (!map.isPositionOccupied(nodePosition))
	{
		auto unit = std::find_if(units.cbegin(), units.cend(), [&nodePosition](const auto& unit) -> bool
		{
			return Globals::convertToNodePosition(unit.getPosition()) == nodePosition;
		});

		if (unit == units.cend())
		{
			auto worker = std::find_if(workers.cbegin(), workers.cend(), [&nodePosition](const auto& harvester) -> bool
			{
				return Globals::convertToNodePosition(harvester.getPosition()) == nodePosition;
			});
			if (worker != workers.cend())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	return false;
}
std::vector<glm::vec3> PathFinding::getFormationPositions(const glm::vec3& startingPosition,
	const std::vector<Unit*>& selectedUnits, const Map& map)
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

glm::vec3 PathFinding::getClosestAvailablePosition(const glm::vec3& startingPosition, const std::list<Unit>& units, 
	const std::list<Worker>& workers, const Map& map)
{
	if (isPositionAvailable(Globals::convertToNodePosition(startingPosition), map, units, workers))
	{
		return startingPosition;
	}	

	m_graph.reset(m_frontier);
	m_frontier.push(Globals::convertToGridPosition(startingPosition));
	glm::ivec2 availablePositionOnGrid = {0, 0};
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions = getAllAdjacentPositions(position, map, units, workers);
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
	glm::vec3 direction = { 0.0f, 0.0f, 0.0f };
	if (entityPosition == centrePositionAABB)
	{
		direction = glm::normalize(glm::vec3(Globals::getRandomNumber(-1.0f, 1.0f), Globals::GROUND_HEIGHT, Globals::getRandomNumber(-1.0f, 1.0f)));
	}
	else
	{
		direction = glm::normalize(entityPosition - centrePositionAABB);
	}

	glm::vec3 position = centrePositionAABB;
	for (float ray = 1.0f; ray <= Globals::NODE_SIZE * 7.0f; ++ray)
	{
		position = position + direction * 1.0f;
		if (!AABB.contains(position) && !map.isPositionOccupied(position) && Globals::isPositionInMapBounds(position))
		{
			closestPosition = position;
			break;
		}
	}

	assert(closestPosition != centrePositionAABB);
	return closestPosition;
}

void PathFinding::getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const GetAllAdjacentPositions& getAdjacentPositions, bool includeWorldDestinationPosition)
{
	assert(getAdjacentPositions && pathToPosition.empty());

	if (unit.getPosition() == destination)
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = glm::distance(destination, unit.getPosition());
	glm::ivec2 closestAvailablePosition = { 0, 0 };
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			if (includeWorldDestinationPosition)
			{
				pathToPosition.push_back(destination);
			}

			getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue);
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
					if (glm::distance(glm::vec2(destinationOnGrid), glm::vec2(adjacentPosition.position)) < shortestDistance)
					{
						closestAvailablePosition = adjacentPosition.position;
						shortestDistance = glm::distance(glm::vec2(destinationOnGrid), glm::vec2(adjacentPosition.position));
					}

					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + glm::distance(glm::vec2(adjacentPosition.position), glm::vec2(currentNode.position)),
						glm::distance(glm::vec2(destinationOnGrid), glm::vec2(adjacentPosition.position)));

					if (m_openQueue.isSuccessorNodeValid(adjacentNode))
					{
						m_openQueue.changeNode(adjacentNode);
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

	if (pathToPosition.empty())
	{	
		getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, closestAvailablePosition, m_closedQueue);
	}
}

void PathFinding::convertPathToWaypoints(std::vector<glm::vec3>& pathToPosition, const Unit& unit, const std::list<Unit>& units,
	const Map& map)
{
	assert(!units.empty());
	if (pathToPosition.size() <= size_t(1))
	{
		return;
	}

	std::queue<glm::vec3> positionsToDelete;
	int startingIndex = static_cast<int>(pathToPosition.size()) - 1;
	int i = startingIndex - 1;
	bool destinationReached = false;
	while (startingIndex > 0 && !destinationReached)
	{
		glm::vec3 startingPosition = pathToPosition[startingIndex];
		glm::vec3 targetPosition = pathToPosition[i];
		glm::vec3 position = startingPosition;
		float MAX_RAY = glm::distance(targetPosition, startingPosition);
		bool collision = false;

		for (float ray = 0.1f; ray <= MAX_RAY; ray += 0.1f)
		{
			position = position + glm::normalize(targetPosition - startingPosition) * 0.1f;

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
			positionsToDelete.push(pathToPosition[i]);
				
			if (i - 1 > 0)
			{
				--i;
			}
			else
			{
				destinationReached = true;
			}
		}
		else
		{
			startingIndex = i;
			i = startingIndex - 1;
		}
	}

	assert(positionsToDelete.size() < pathToPosition.size());
	while (!positionsToDelete.empty())
	{
		const glm::vec3& positionToDelete = positionsToDelete.front();
		auto iter = std::find_if(pathToPosition.begin(), pathToPosition.end(), [&positionToDelete](const auto& position)
		{
			return position == positionToDelete;
		});
		assert(iter != pathToPosition.end());
			
		pathToPosition.erase(iter);
		positionsToDelete.pop();
	}
}