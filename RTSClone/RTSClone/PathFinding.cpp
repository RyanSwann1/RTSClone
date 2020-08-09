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
						//glm::vec2 direction = glm::normalize(glm::vec2(convertToGridPosition(otherUnit.getPosition()) - position));

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

	const auto nodeCompare = [](const auto& a, const auto& b) -> bool { return b.getF() < a.getF(); };
	struct NodePriorityQueue : public std::priority_queue<GraphNodeAStar, std::vector<GraphNodeAStar>, decltype(nodeCompare)>
	{
		NodePriorityQueue()
			: priority_queue(nodeCompare)
		{}
		NodePriorityQueue(size_t size)
			: priority_queue(nodeCompare)
		{
			c.reserve(size);
		}

		void clear()
		{
			c.clear();
		}

		bool contains(const glm::ivec2& position) const
		{
			auto cIter = std::find_if(c.cbegin(), c.cend(), [&position](const auto& node) -> bool
			{
				return position == node.position;
			});

			return cIter != c.cend();
		}

		GraphNodeAStar& getNode(const glm::ivec2& position)
		{
			auto iter = std::find_if(c.begin(), c.end(), [&position](const auto& node) -> bool
			{
				return node.position == position;
			});

			assert(iter != c.end());
			return (*iter);
		}

		bool isSuccessorNodeValid(const GraphNodeAStar& successorNode) const
		{
			auto matchingNode = std::find_if(c.cbegin(), c.cend(), [&successorNode](const auto& node) -> bool
			{
				return successorNode.position == node.position;
			});
			if (matchingNode != c.cend() && successorNode.getF() < matchingNode->getF())
			{
				return true;
			}

			return false;
		}
	};
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
	m_frontier()
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

void PathFinding::getPathToPosition(const Unit& unit, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition,
	const Map& map, const std::vector<Unit>& units)
{
	assert(pathToPosition.empty());
	reset();
	glm::ivec2 destinationPositionOnGrid = convertToGridPosition(destinationPosition);
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	m_frontier.push(startingPositionOnGrid);
	bool destinationReached = false;
	glm::ivec2 position = { 0, 0 };
	glm::ivec2 shortestDistancePosition = { 0, 0 };
	while (!m_frontier.empty() && !destinationReached)
	{
		position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, map, units, unit);
		float distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(startingPositionOnGrid));
		shortestDistancePosition = position;
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid && glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position)) < distance)
			{
				distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position));
				shortestDistancePosition = adjacentPosition.position;
			}
		}

		//If Shortest Position found
		if (!m_graph.isPositionVisited(shortestDistancePosition))
		{
			m_graph.addToGraph(shortestDistancePosition, position);
			m_frontier.push(shortestDistancePosition);

			if (shortestDistancePosition == destinationPositionOnGrid)
			{
				destinationReached = true;
				getPathFromVisitedNodes(startingPositionOnGrid, shortestDistancePosition, destinationPosition, pathToPosition, m_graph);
			}
		}
		//If shortest position is not found
		else
		{
			for (const auto& adjacentPosition : adjacentPositions)
			{
				if (adjacentPosition.valid)
				{
					if (!m_graph.isPositionVisited(adjacentPosition.position))
					{
						m_graph.addToGraph(adjacentPosition.position, position);
						m_frontier.push(adjacentPosition.position);
					}

					if (adjacentPosition.position == destinationPositionOnGrid)
					{
						destinationReached = true;
						getPathFromVisitedNodes(startingPositionOnGrid, adjacentPosition.position, destinationPosition, pathToPosition, m_graph);
						break;
					}
				}
			}
		}

		assert(m_frontier.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	if(pathToPosition.empty())
	{
		std::cout << "Path is empty\n";
	}
}

void PathFinding::getPathToPosition(const glm::vec3& startingPosition, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, const Map& map)
{
	assert(pathToPosition.empty());
	reset();
	glm::ivec2 destinationPositionOnGrid = convertToGridPosition(destination);
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(startingPosition);
	m_frontier.push(startingPositionOnGrid);
	bool destinationReached = false;

	while (!m_frontier.empty() && !destinationReached)
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
				}

				if (adjacentPosition.position == destinationPositionOnGrid)
				{
					getPathFromVisitedNodes(startingPositionOnGrid, destinationPositionOnGrid, destination, pathToPosition, m_graph);
					destinationReached = true;
					break;
				}
			}
		}

		assert(m_frontier.size() < Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	if (pathToPosition.empty())
	{
		std::cout << "Empty Path\n";
	}
}

void PathFinding::getPathToPositionAmongstGroup(const Unit& unit, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const std::vector<Unit>& units, const std::vector<const Unit*>& selectedUnits)
{
	assert(pathToPosition.empty());
	reset();
	glm::ivec2 destinationPositionOnGrid = convertToGridPosition(destinationPosition);
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	m_frontier.push(startingPositionOnGrid);
	bool destinationReached = false;
	glm::ivec2 position = { 0, 0 };
	while (!m_frontier.empty() && !destinationReached)
	{
		position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, map, units, unit, selectedUnits);
		float distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(startingPositionOnGrid));
		glm::ivec2 shortestDistancePosition = position;
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid && glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position)) < distance)
			{
				distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position));
				shortestDistancePosition = adjacentPosition.position;
			}
		}

		//If Shortest Position found
		if (!m_graph.isPositionVisited(shortestDistancePosition))
		{
			m_graph.addToGraph(shortestDistancePosition, position);
			m_frontier.push(shortestDistancePosition);

			if (shortestDistancePosition == destinationPositionOnGrid)
			{
				destinationReached = true;
				getPathFromVisitedNodes(startingPositionOnGrid, shortestDistancePosition, destinationPosition, pathToPosition, m_graph);
			}
		}
		else
		{
			getPathFromVisitedNodes(startingPositionOnGrid, shortestDistancePosition, pathToPosition, m_graph);
			destinationReached = true;
		}

		assert(m_frontier.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	if (pathToPosition.empty())
	{
		std::cout << "Path is empty\n";
	}
}

void PathFinding::getPathToPositionAmongstGroup(const Unit& unit, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition, const Map& map)
{
	assert(pathToPosition.empty());
	reset();
	glm::ivec2 destinationPositionOnGrid = convertToGridPosition(destinationPosition);
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	m_frontier.push(startingPositionOnGrid);
	bool destinationReached = false;
	glm::ivec2 position = { 0, 0 };
	while (!m_frontier.empty() && !destinationReached)
	{
		position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, map);
		float distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(startingPositionOnGrid));
		glm::ivec2 shortestDistancePosition = position;
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid && glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position)) < distance)
			{
				distance = glm::distance(glm::vec2(destinationPositionOnGrid), glm::vec2(adjacentPosition.position));
				shortestDistancePosition = adjacentPosition.position;
			}
		}

		//If Shortest Position found
		if (!m_graph.isPositionVisited(shortestDistancePosition))
		{
			m_graph.addToGraph(shortestDistancePosition, position);
			m_frontier.push(shortestDistancePosition);

			if (shortestDistancePosition == destinationPositionOnGrid)
			{
				destinationReached = true;
				getPathFromVisitedNodes(startingPositionOnGrid, shortestDistancePosition, destinationPosition, pathToPosition, m_graph);
			}
		}
		else
		{
			getPathFromVisitedNodes(startingPositionOnGrid, shortestDistancePosition, pathToPosition, m_graph);
			destinationReached = true;
		}

		assert(m_frontier.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	if (pathToPosition.empty())
	{
		std::cout << "Path is empty\n";
	}
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
			getPathToPosition(centrePositionAABB, position, pathToPosition, map);
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

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = convertToGridPosition(destination);

	NodePriorityQueue openQueue;
	NodePriorityQueue closedQueue;

	openQueue.emplace(startingPositionOnGrid, startingPositionOnGrid, 0.0f, glm::distance(glm::vec2(destinationOnGrid), glm::vec2(startingPositionOnGrid)));

	while (!openQueue.empty() && !destinationReached)
	{
		GraphNodeAStar currentNode = openQueue.top();
		openQueue.pop();

		if (currentNode.position == destinationOnGrid)
		{
			pathToPosition.emplace_back(convertToWorldPosition(currentNode.position));

			glm::ivec2 parentPosition = currentNode.parentPosition;
			while (parentPosition != startingPositionOnGrid)
			{
				const GraphNodeAStar& parentNode = closedQueue.getNode(parentPosition);
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
				if (!adjacentPosition.valid || closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else if(adjacentPosition.valid)
				{
					GraphNodeAStar adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + glm::distance(glm::vec2(adjacentPosition.position), glm::vec2(currentNode.position)),
						glm::distance(glm::vec2(destinationOnGrid), glm::vec2(adjacentPosition.position)));

					if (openQueue.isSuccessorNodeValid(adjacentNode))
					{
						openQueue.getNode(adjacentPosition.position) = adjacentNode;
					}
					else if (!openQueue.contains(adjacentPosition.position))
					{
						openQueue.push(adjacentNode);
					}
				}
			}
		}
		
		closedQueue.push(currentNode);
		assert(openQueue.size() < Globals::MAP_SIZE * Globals::MAP_SIZE && closedQueue.size() < Globals::MAP_SIZE * Globals::MAP_SIZE);
	}
}

//GraphNodeAStar
GraphNodeAStar::GraphNodeAStar(const glm::ivec2& position, const glm::ivec2& parentPosition, float g, float h)
	: traversable(true),
	position(position),
	parentPosition(parentPosition),
	g(g),
	h(h)
{}

float GraphNodeAStar::getF() const
{
	return h + g;
}