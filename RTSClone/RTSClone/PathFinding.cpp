#include "PathFinding.h"
#include "Globals.h"
#include "Map.h"
#include "Unit.h"
#include "Harvester.h"
#include "ModelManager.h"
#include <limits>
#include <queue>

namespace
{
	struct AdjacentPosition
	{
		AdjacentPosition()
			: valid(false),
			unitCollision(false),
			position()
		{}	
		AdjacentPosition(const glm::ivec2& position)
			: valid(true),
			unitCollision(false),
			position(position)
		{}
		AdjacentPosition(const glm::ivec2& position, bool valid, bool unitCollision)
			: valid(valid),
			unitCollision(unitCollision),
			position(position)
		{}

		bool valid;
		bool unitCollision;
		glm::ivec2 position;
	};

	bool isPositionInMapBounds(const glm::ivec2& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE;
	}

	bool isPositionWithinNodeSize(const glm::ivec2& nodePosition, const glm::ivec2& position)
	{
		return position.x >= nodePosition.x &&
			position.x <= nodePosition.x + Globals::NODE_SIZE &&
			position.y >= nodePosition.y &&
			position.y <= nodePosition.y + Globals::NODE_SIZE;
	}

	glm::vec3 convertToWorldPosition(const glm::ivec2& position)
	{
		return { position.x * Globals::NODE_SIZE, Globals::GROUND_HEIGHT, position.y * Globals::NODE_SIZE };
	}

	glm::ivec2 convertToGridPosition(const glm::vec3& position)
	{
		return { position.x / Globals::NODE_SIZE, position.z / Globals::NODE_SIZE };
	}

	constexpr std::array<glm::ivec2, 8> ALL_DIRECTIONS =
	{
		glm::ivec2(0, 1),
		glm::ivec2(1, 1),
		glm::ivec2(1, 0),
		glm::ivec2(1, -1),
		glm::ivec2(0, -1),
		glm::ivec2(-1, -1),
		glm::ivec2(-1, 0),
		glm::ivec2(-1, 1)
	};

	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map,
		const std::vector<Unit>& units)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS[i];
			if (isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
			{
				bool unitCollision = false;
				for (const auto& otherUnit : units)
				{
					if (otherUnit.getAABB().contains(convertToWorldPosition(adjacentPosition)))
					{
						unitCollision = true;
						break;
					}
				}

				if (!unitCollision)
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition, true, unitCollision);
				}
				else
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition, true, unitCollision);
				}
			}
		}

		return adjacentPositions;
	}
	
	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositionsAStar(const glm::ivec2& position, const Map& map,
		const std::vector<Unit>& units, const Unit& unit)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS[i];
			if (isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
			{
				bool unitCollision = false;
				for (const auto& otherUnit : units)
				{
					if (&otherUnit != &unit && otherUnit.getCurrentState() == eUnitState::Idle)
					{
						if (otherUnit.getAABB().contains(convertToWorldPosition(adjacentPosition)))
						{
							unitCollision = true;
							break;
						}
					}
				}

				if (!unitCollision)
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition);
				}
			}
		}

		return adjacentPositions;
	}

	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map,
		const std::vector<Unit>& units, const Unit& unit, const std::vector<const Unit*>& selectedUnits)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS[i];
			if (isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
			{
				bool unitCollision = false;
				for (const auto& otherUnit : units)
				{
					if (&otherUnit != &unit && 
						std::find(selectedUnits.cbegin(), selectedUnits.cend(), &otherUnit) == selectedUnits.cend() &&
						otherUnit.getCurrentState() == eUnitState::Idle)
					{
						if (otherUnit.getAABB().contains(convertToWorldPosition(adjacentPosition)))
						{
							unitCollision = true;
							break;
						}
					}
				}

				if (!unitCollision)
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition);
				}
			}
		}

		return adjacentPositions;
	}

	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map, 
		const std::vector<Unit>& units, const Unit& unit)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS[i];
			if (isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
			{
				bool unitCollision = false;
				for (const auto& otherUnit : units)
				{
					if (&otherUnit != &unit && otherUnit.getCurrentState() == eUnitState::Idle)
					{
						glm::vec2 direction = glm::normalize(glm::vec2(convertToGridPosition(otherUnit.getPosition()) - position));
						if (otherUnit.getAABB().contains(convertToWorldPosition(glm::vec2(adjacentPosition.x, adjacentPosition.y) + direction * 3.0f)))
						{
							unitCollision = true;
							break;
						}

						if (otherUnit.getAABB().contains(convertToWorldPosition(adjacentPosition)))
						{
							unitCollision = true;
							break;
						}
					}
				}

				if (!unitCollision)
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition);
				}
			}
		}

		return adjacentPositions;
	}

	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS[i];
			if (isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}
		}

		return adjacentPositions;
	}

	void getPathFromVisitedNodes(const glm::ivec2& startingPosition, const glm::ivec2& destinationPositionOnGrid, 
		std::vector<glm::vec3>& pathToPosition, Graph& graph)
	{
		pathToPosition.push_back(convertToWorldPosition(destinationPositionOnGrid));
		glm::ivec2 position = graph.getPreviousPosition(destinationPositionOnGrid);

		while (position != startingPosition)
		{
			pathToPosition.push_back(convertToWorldPosition(position));
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
			glm::vec3 position = convertToWorldPosition(positionOnGrid);
			position.x += static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			pathToPosition.push_back(convertToWorldPosition(positionOnGrid));
			positionOnGrid = graph.getPreviousPosition(positionOnGrid);

			assert(pathToPosition.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
		}
	}
}

//Graph Node
GraphNode::GraphNode()
	: cameFrom(),
	visited(false)
{}

GraphNode::GraphNode(const glm::ivec2& cameFrom)
	: cameFrom(cameFrom),
	visited(true)
{}

const glm::ivec2& GraphNode::getCameFrom() const
{
	return cameFrom;
}

bool GraphNode::isVisited() const
{
	return visited;
}

//Graph
Graph::Graph()
	: m_graph()
{}

void Graph::addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition)
{
	assert(isPositionInMapBounds(position) && !m_graph[Globals::convertTo1D(position)].isVisited());
	if (isPositionInMapBounds(position) && !m_graph[Globals::convertTo1D(position)].isVisited())
	{
		m_graph[Globals::convertTo1D(position)] = GraphNode(cameFromPosition);
	}
}

bool Graph::isEmpty() const
{
	return m_graph.empty();
}

const glm::ivec2& Graph::getPreviousPosition(const glm::ivec2& position) const
{
	assert(isPositionInMapBounds(position) && m_graph[Globals::convertTo1D(position)].isVisited());
	if (isPositionInMapBounds(position) && m_graph[Globals::convertTo1D(position)].isVisited())
	{
		return m_graph[Globals::convertTo1D(position)].getCameFrom();
	}
}

bool Graph::isPositionVisited(const glm::ivec2& position) const
{
	assert(isPositionInMapBounds(position));
	if (isPositionInMapBounds(position))
	{
		return m_graph[Globals::convertTo1D(position)].isVisited();
	}
}

void Graph::resetGraph()
{
	for (auto& i : m_graph)
	{
		i = GraphNode();
	}
}

//PathFinding
PathFinding::PathFinding()
	: m_graph(),
	m_frontier(),
	m_openQueue(static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)),
	m_closedQueue(static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE))
{}

void PathFinding::reset()
{
	m_graph.resetGraph();
	std::queue<glm::ivec2> empty;
	m_frontier.swap(empty);
}

std::vector<glm::vec3> PathFinding::getFormationPositions(const glm::vec3& startingPosition,
	const std::vector<const Unit*> selectedUnits, const Map& map)
{
	//TODO: Sort by closest
	assert(!selectedUnits.empty() && std::find(selectedUnits.cbegin(), selectedUnits.cend(), nullptr) == selectedUnits.cend());
	reset();

	std::vector<glm::vec3> unitFormationPositions;
	unitFormationPositions.reserve(selectedUnits.size());
	int selectedUnitIndex = 0;
	m_frontier.push(convertToGridPosition(startingPosition));

	while (!m_frontier.empty() && unitFormationPositions.size() < selectedUnits.size())
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, map);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid)
			{
				if (!m_graph.isPositionVisited(adjacentPosition.position))
				{
					m_graph.addToGraph(adjacentPosition.position, position);
					m_frontier.push(adjacentPosition.position);

					assert(selectedUnitIndex < selectedUnits.size());
					unitFormationPositions.emplace_back(convertToWorldPosition(adjacentPosition.position));
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
	if (isPositionInMapBounds(startingPositionOnGrid) && !map.isPositionOccupied(startingPositionOnGrid))
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

	reset();
	m_frontier.push(startingPositionOnGrid);
	glm::ivec2 availablePositionOnGrid = {0, 0};
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, map, units);
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

	return convertToWorldPosition(availablePositionOnGrid);
}

void PathFinding::getPathToClosestPositionOutsideAABB(const glm::vec3& entityPosition, const AABB& AABB, const glm::vec3& centrePositionAABB, 
	const Map& map, std::vector<glm::vec3>& pathToPosition)
{
	glm::vec3 position = centrePositionAABB;
	glm::vec3 direction = glm::normalize(entityPosition - centrePositionAABB);
	for (float ray = 1.0f; ray <= Globals::NODE_SIZE * 5.0f; ++ray)
	{
		position = position + direction * ray;
		if (!AABB.contains(position) && !map.isPositionOccupied(position) && Globals::isPositionInMapBounds(position))
		{
			getPathToPositionAStar(centrePositionAABB, position, pathToPosition, map);
			if (!pathToPosition.empty())
			{
				return;
			}
		}
	}
	
	assert(false);
}

void PathFinding::getPathToPositionAStar(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, const Map& map,
	const std::vector<Unit>& units)
{
	assert(!units.empty());

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = convertToGridPosition(destination);

	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			pathToPosition.emplace_back(convertToWorldPosition(currentNode.position));

			glm::ivec2 parentPosition = currentNode.parentPosition;
			while (parentPosition != startingPositionOnGrid)
			{
				const PriorityQueueNode& parentNode = m_closedQueue.getNode(parentPosition);
				parentPosition = parentNode.parentPosition;

				glm::vec3 position = convertToWorldPosition(parentNode.position);
				pathToPosition.emplace_back(position);
			}
			
			destinationReached = true;
		}
		else
		{ 
			std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositionsAStar(currentNode.position, map, units, unit);
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

void PathFinding::getPathToPositionAStar(const glm::vec3& startingPosition, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, const Map& map)
{
	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(startingPosition);
	glm::ivec2 destinationOnGrid = convertToGridPosition(destination);

	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			pathToPosition.emplace_back(convertToWorldPosition(currentNode.position));

			glm::ivec2 parentPosition = currentNode.parentPosition;
			while (parentPosition != startingPositionOnGrid)
			{
				const PriorityQueueNode& parentNode = m_closedQueue.getNode(parentPosition);
				parentPosition = parentNode.parentPosition;

				glm::vec3 position = convertToWorldPosition(parentNode.position);
				pathToPosition.emplace_back(position);
			}

			destinationReached = true;
		}
		else
		{
			std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(currentNode.position, map);
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

void PathFinding::getPathToPositionAStar(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const std::vector<Unit>& units, const std::vector<const Unit*>& selectedUnits)
{
	assert(!units.empty() && !selectedUnits.empty());

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = convertToGridPosition(destination);

	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			pathToPosition.emplace_back(convertToWorldPosition(currentNode.position));

			glm::ivec2 parentPosition = currentNode.parentPosition;
			while (parentPosition != startingPositionOnGrid)
			{
				const PriorityQueueNode& parentNode = m_closedQueue.getNode(parentPosition);
				parentPosition = parentNode.parentPosition;

				glm::vec3 position = convertToWorldPosition(parentNode.position);
				pathToPosition.emplace_back(position);
			}

			destinationReached = true;
		}
		else
		{
			std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(currentNode.position, map, units, unit, selectedUnits);
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
