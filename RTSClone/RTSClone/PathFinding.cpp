#include "PathFinding.h"
#include "Globals.h"
#include "Map.h"
#include "Unit.h"
#include "Worker.h"
#include "ModelManager.h"
#include "AdjacentPositions.h"
#include "Faction.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "FactionHandler.h"
#include "FactionAI.h"
#include "Base.h"
#include <limits>
#include <queue>
#include <random>

namespace
{
	bool isPriorityQueueWithinSizeLimit(const PriorityQueue& priorityQueue, const glm::ivec2& mapSize)
	{
		return static_cast<int>(priorityQueue.getSize()) <= mapSize.x * mapSize.y;
	}

	bool isFrontierWithinSizeLimit(const std::queue<glm::ivec2>& frontier, const glm::ivec2& mapSize)
	{
		return static_cast<int>(frontier.size()) <= mapSize.x * mapSize.y;
	}

	bool isPathWithinSizeLimit(const std::vector<glm::vec3>& pathToPosition, const glm::ivec2& mapSize)
	{
		return static_cast<int>(pathToPosition.size()) <= mapSize.x * mapSize.y;
	}

	void getPathFromClosedQueue(std::vector<glm::vec3>& pathToPosition, const glm::ivec2& startingPositionOnGrid,
		const PriorityQueueNode& startingNode, const PriorityQueue& closedQueue, const Map& map)
	{
		pathToPosition.push_back(Globals::convertToWorldPosition(startingNode.position));
		glm::ivec2 parentPosition = startingNode.parentPosition;

		while (parentPosition != startingPositionOnGrid)
		{
			const PriorityQueueNode& parentNode = closedQueue.getNode(parentPosition);
			parentPosition = parentNode.parentPosition;

			pathToPosition.push_back(Globals::convertToWorldPosition(parentNode.position));
			assert(isPathWithinSizeLimit(pathToPosition, map.getSize()));
		}
	}

	void getPathFromClosedQueue(std::vector<glm::vec3>& pathToPosition, const glm::ivec2& startingPositionOnGrid,
		const glm::ivec2 startingPosition, const PriorityQueue& closedQueue, const Map& map)
	{
		pathToPosition.push_back(Globals::convertToWorldPosition(startingPosition));
		glm::ivec2 parentPosition = closedQueue.getNode(startingPosition).parentPosition;

		while (parentPosition != startingPositionOnGrid)
		{
			const PriorityQueueNode& parentNode = closedQueue.getNode(parentPosition);
			parentPosition = parentNode.parentPosition;

			pathToPosition.push_back(Globals::convertToWorldPosition(parentNode.position));
			assert(isPathWithinSizeLimit(pathToPosition, map.getSize()));
		}
	}

	void addToOpenQueue(PriorityQueue& openQueue, const PriorityQueueNode& node)
	{
		if (openQueue.isSuccessorNodeValid(node))
		{
			openQueue.changeNode(node);
		}
		else if (!openQueue.contains(node.position))
		{
			openQueue.add(node);
		}
	}

	bool isWithinBuildingPositionsRange(const std::vector<glm::vec3>& buildPositions, const glm::vec3& position)
	{
		auto buildPosition = std::find_if(buildPositions.cbegin(), buildPositions.cend(), [&position](const auto& buildPosition)
		{
			return Globals::getSqrDistance(buildPosition, position) < static_cast<float>(Globals::NODE_SIZE) * 4.0f;
		});

		return buildPosition != buildPositions.cend();
	}
}

//ThetaStarGraphNode
ThetaStarGraphNode::ThetaStarGraphNode()
	: position(0, 0),
	cameFrom(0, 0),
	g(0.f),
	h(0.f)
{}

ThetaStarGraphNode::ThetaStarGraphNode(glm::ivec2 position, glm::ivec2 cameFrom, float g, float h)
	: position(position),
	cameFrom(cameFrom),
	g(g),
	h(h)
{}

float ThetaStarGraphNode::getF() const
{
	return g + h;
}

//PathFinding
PathFinding::PathFinding()
	: m_sharedContainer(),
	m_graph(),
	m_frontier(),
	m_openQueue(),
	m_closedQueue(),
	m_thetaGraph(),
	m_newFrontier()
{
	subscribeToMessenger<GameMessages::NewMapSize>(
		[this](const GameMessages::NewMapSize& gameMessage) { return onNewMapSize(gameMessage); }, this);
}

PathFinding::~PathFinding()
{
	//Currently exists within static memory
	//Don't unsub
	//GameMessenger::getInstance().unsubscribe<GameMessages::NewMapSize>(this);
}

void PathFinding::onNewMapSize(const GameMessages::NewMapSize& gameMessage)
{
	m_sharedContainer.clear();
	m_sharedContainer.reserve(
		static_cast<size_t>(gameMessage.mapSize.x) * static_cast<size_t>(gameMessage.mapSize.y));

	m_thetaGraph.clear();
	m_thetaGraph.resize(
		static_cast<size_t>(gameMessage.mapSize.x) * static_cast<size_t>(gameMessage.mapSize.y));
}


bool PathFinding::isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType buildingEntityType, const Map& map, 
	glm::vec3& buildPosition, const FactionAI& owningFaction, const BaseHandler& baseHandler)
{
	m_sharedContainer.clear();
	AABB buildingAABB(startingPosition, ModelManager::getInstance().getModel(buildingEntityType));
	for (int i = 0; i < 5; ++i)
	{
		m_graph.reset(m_frontier);
		m_frontier = std::queue<glm::ivec2>();
		m_frontier.emplace(Globals::convertToGridPosition(startingPosition));
		bool buildPositionFound = false;
		while (!m_frontier.empty() && !buildPositionFound)
		{
			glm::ivec2 position = m_frontier.front();
			m_frontier.pop();

			for (const auto& adjacentPosition : getAllAdjacentPositions(position, map))
			{
				if (adjacentPosition.valid)
				{
					buildingAABB.update(Globals::convertToWorldPosition(adjacentPosition.position));
					if (!map.isAABBOccupied(buildingAABB) &&
						!owningFaction.isWithinRangeOfBuildings(Globals::convertToWorldPosition(adjacentPosition.position), Globals::NODE_SIZE * 4.0f) && 
						!baseHandler.isWithinRangeOfMinerals(Globals::convertToWorldPosition(adjacentPosition.position), Globals::NODE_SIZE * 6.0f) &&
						!isWithinBuildingPositionsRange(m_sharedContainer, Globals::convertToWorldPosition(adjacentPosition.position)))
					{
						buildPositionFound = true;
						m_sharedContainer.emplace_back(Globals::convertToWorldPosition(adjacentPosition.position));
						break;
					}
				}
				if (!m_graph.isPositionVisited(adjacentPosition.position, map))
				{
					m_graph.addToGraph(adjacentPosition.position, position, map);
					m_frontier.push(adjacentPosition.position);
				}
			}
		}
	}

	if (!m_sharedContainer.empty())
	{
		buildPosition = m_sharedContainer[Globals::getRandomNumber(0, static_cast<int>(m_sharedContainer.size()) - 1)];
		return true;
	}

	return false;
}

bool PathFinding::isPositionInLineOfSight(const glm::vec3& startingPosition, const glm::vec3& targetPosition, const Map& map) const
{
	glm::vec3 startingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(startingPosition));
	glm::vec3 endingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(targetPosition));
	glm::vec3 direction = glm::normalize(endingCenteredPosition - startingCenteredPosition);
	bool targetEntityVisible = true;

	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(
		glm::ceil(glm::distance(endingCenteredPosition, startingCenteredPosition))); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = startingCenteredPosition + direction * static_cast<float>(i);
		if (map.isPositionOccupied(position))
		{
			targetEntityVisible = false;
			break;
		}
	}

	return targetEntityVisible;
}

bool PathFinding::isPositionInLineOfSight(const glm::vec3& startingPosition, const glm::vec3& targetPosition, const Map& map, const Unit& unit) const
{
	glm::vec3 startingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(startingPosition));
	glm::vec3 endingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(targetPosition));
	glm::vec3 direction = glm::normalize(endingCenteredPosition - startingCenteredPosition);
	float distance = glm::distance(endingCenteredPosition, startingCenteredPosition);
	bool targetEntityVisible = true;

	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = startingCenteredPosition + direction * static_cast<float>(i);
		if (map.isPositionOccupied(position) || 
			!map.isPositionOnUnitMapAvailable(Globals::convertToGridPosition(position), unit.getID()))
		{
			targetEntityVisible = false;
			break;
		}
	}

	return targetEntityVisible;
}

bool PathFinding::isPositionInLineOfSight(glm::ivec2 startingPositionOnGrid, glm::ivec2 targetPositionOnGrid, const Map& map, const Unit& unit) const
{
	 glm::vec3 startingPosition = Globals::convertToWorldPosition(startingPositionOnGrid);
	 glm::vec3 targetPosition = Globals::convertToWorldPosition(targetPositionOnGrid);
	 glm::vec3 direction = glm::normalize(targetPosition - startingPosition);
	 float distance = glm::distance(targetPosition, startingPosition);

	 for (int i = Globals::NODE_SIZE; i <= static_cast<int>(
		 glm::ceil(glm::distance(targetPosition, startingPosition))); i += Globals::NODE_SIZE)
	 {
		 glm::vec3 position = startingPosition + direction * static_cast<float>(i);
		 if (map.isPositionOccupied(position) ||
			 !map.isPositionOnUnitMapAvailable(Globals::convertToGridPosition(position), unit.getID()))
		 {
			return false;
		 }
	 }

	 return true;
}

bool PathFinding::isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map) const
{
	glm::vec3 startingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(startingPosition));
	glm::vec3 endingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(targetEntity.getPosition()));
	glm::vec3 direction = glm::normalize(endingCenteredPosition - startingCenteredPosition);
	float distance = glm::distance(endingCenteredPosition, startingCenteredPosition); 
	bool targetEntityVisible = true;

	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = startingCenteredPosition + direction * static_cast<float>(i);
		if (targetEntity.getAABB().contains(position))
		{
			break;
		}
		else if (map.isPositionOccupied(position))
		{
			targetEntityVisible = false;
			break;
		}
	}

	return targetEntityVisible;
}

bool PathFinding::isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map, const AABB& senderAABB) const
{
	glm::vec3 startingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(startingPosition));
	glm::vec3 endingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(targetEntity.getPosition()));
	glm::vec3 direction = glm::normalize(endingCenteredPosition - startingCenteredPosition);
	float distance = glm::distance(endingCenteredPosition, startingCenteredPosition);
	bool targetEntityVisible = true;

	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = startingCenteredPosition + direction * static_cast<float>(i);
		if (targetEntity.getAABB().contains(position))
		{
			break;
		}
		else if (!senderAABB.contains(position) && map.isPositionOccupied(position))
		{
			targetEntityVisible = false;
			break;
		}
	}

	return targetEntityVisible;
}

bool PathFinding::isTargetInLineOfSight(const Unit& unit, const Entity& targetEntity, const Map& map) const
{
	glm::vec3 startingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(unit.getPosition()));
	glm::vec3 endingCenteredPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(targetEntity.getPosition()));
	glm::vec3 direction = glm::normalize(endingCenteredPosition - startingCenteredPosition);
	float distance = glm::distance(endingCenteredPosition, startingCenteredPosition);
	bool targetEntityVisible = true;

	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = startingCenteredPosition + direction * static_cast<float>(i);
		if (targetEntity.getAABB().contains(position))
		{
			break;
		}
		else if (!map.isPositionOnUnitMapAvailable(Globals::convertToGridPosition(position), unit.getID()) && map.isPositionOccupied(position))
		{
			targetEntityVisible = false;
			break;
		}
	}

	return targetEntityVisible;
}

bool PathFinding::getClosestAvailableEntitySpawnPosition(const EntitySpawnerBuilding& building, const std::vector<std::unique_ptr<Unit>>& units, 
	const std::vector<std::unique_ptr<Worker>>& workers, const Map& map, glm::vec3& spawnPosition)
{
	m_graph.reset(m_frontier);
	m_frontier.push(Globals::convertToGridPosition(building.getPosition()));
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		for (const auto& adjacentPosition : getAdjacentPositions(position, map, units, workers, building.getAABB()))
		{
			if (adjacentPosition.valid && !building.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition.position)))
			{
				spawnPosition = Globals::convertToWorldPosition(adjacentPosition.position);
				availablePositionFound = true;
				break;
			}
			else if (!m_graph.isPositionVisited(adjacentPosition.position, map))
			{
				m_graph.addToGraph(adjacentPosition.position, position, map);
				m_frontier.push(adjacentPosition.position);
			}	
		}

		assert(isFrontierWithinSizeLimit(m_frontier, map.getSize()));
	}

	return availablePositionFound;
}

bool PathFinding::getRandomPositionOutsideAABB(const Entity& building, const Map& map, glm::vec3& positionOutsideAABB)
{
	m_graph.reset(m_frontier);
	m_frontier.push(Globals::convertToGridPosition(building.getPosition()));
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		for (const auto& adjacentPosition : getRandomAdjacentPositions(position, map, building.getAABB()))
		{
			if (adjacentPosition.valid && !building.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition.position)))
			{
				positionOutsideAABB = Globals::convertToWorldPosition(adjacentPosition.position);
				availablePositionFound = true;
				break;
			}
			else if (!m_graph.isPositionVisited(adjacentPosition.position, map))
			{
				m_graph.addToGraph(adjacentPosition.position, position, map);
				m_frontier.push(adjacentPosition.position);
			}
		}

		assert(isFrontierWithinSizeLimit(m_frontier, map.getSize()));
	}

	return availablePositionFound;
}

glm::vec3 PathFinding::getClosestPositionToAABB(const glm::vec3& entityPosition, const AABB& AABB, const Map& map)
{
	glm::vec3 centrePositionAABB = AABB.getCenterPosition();
	glm::vec3 direction = glm::normalize(entityPosition - centrePositionAABB);
	assert(entityPosition != centrePositionAABB);

	glm::vec3 closestPosition = centrePositionAABB;
	glm::vec3 position = centrePositionAABB;
	for (int ray = 1; ray <= Globals::NODE_SIZE * 7; ++ray)
	{
		position = position + direction * 1.0f;
		if (!AABB.contains(position) && !map.isPositionOccupied(position) && map.isWithinBounds(position))
		{
			closestPosition = position;
			break;
		}
	}

	assert(closestPosition != centrePositionAABB);
	return closestPosition;
}

bool PathFinding::setUnitAttackPosition(const Unit& unit, const Entity& targetEntity, std::vector<glm::vec3>& pathToPosition,
	const Map& map, FactionHandler& factionHandler)
{
	assert(unit.getID() != targetEntity.getID());

	pathToPosition.clear();
	m_openQueue.clear();
	m_closedQueue.clear();

	glm::ivec2 startingPosition = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 targetPosition = Globals::convertToGridPosition(targetEntity.getPosition());
	bool positionFound = false;

	m_openQueue.add({ startingPosition, startingPosition, 0.0f, Globals::getSqrDistance(targetEntity.getPosition(), unit.getPosition()) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!positionFound && !m_openQueue.isEmpty())
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (Globals::getSqrDistance(targetEntity.getPosition(), Globals::convertToWorldPosition(currentNode.position)) <=
			unit.getAttackRange() * unit.getAttackRange() &&
			isTargetInLineOfSight(unit, targetEntity, map))
		{
			if (currentNode.position == startingPosition)
			{
				if (map.isPositionOnUnitMapAvailable(startingPosition, unit.getID()))
				{
					positionFound = true;
					if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPosition));
					}
				}
			}
			else
			{
				positionFound = true;
				if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
				{
					getPathFromClosedQueue(pathToPosition, startingPosition, currentNode, m_closedQueue, map);
				}
			}
		}
		else
		{
			for (const auto& adjacentPosition : getAdjacentPositions(currentNode.position, map, factionHandler, unit, targetEntity.getAABB()))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(targetPosition, adjacentPosition.position);
					if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
						Globals::convertToWorldPosition(parentNode.position), map, unit))
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
							parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
							sqrDistance);

						addToOpenQueue(m_openQueue, adjacentNode);
					}
					else
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
							currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
							sqrDistance);

						parentNode = currentNode;
						addToOpenQueue(m_openQueue, adjacentNode);
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	return positionFound;
}

void PathFinding::getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, FactionHandler& factionHandler, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (unit.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return;
	}

	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	std::fill(m_thetaGraph.begin(), m_thetaGraph.end(), ThetaStarGraphNode());
	m_newFrontier.clear();
	m_newFrontier.add({startingPositionOnGrid, startingPositionOnGrid, 0.f, Globals::getDistance(destinationOnGrid, startingPositionOnGrid)});

	bool destinationReached = false;

	while (!destinationReached && !m_newFrontier.isEmpty())
	{
		MinHeapNode currentNode = m_newFrontier.pop();
		if (currentNode.position == destinationOnGrid)
		{
			glm::ivec2 position = currentNode.position;
			if (currentNode.position == startingPositionOnGrid)
			{
				if (map.isPositionOnUnitMapAvailable(startingPositionOnGrid, unit.getID()))
				{
					destinationReached = true;
					if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPositionOnGrid));
					}
				}
			}
			else
			{
				destinationReached = true;
				while (position != startingPositionOnGrid)
				{
					pathToPosition.push_back(Globals::convertToWorldPosition(position));
					position = m_thetaGraph[Globals::convertTo1D(position, map.getSize())].cameFrom;

					assert(isPathWithinSizeLimit(pathToPosition, map.getSize()));
				}
			}

			return;
		}

		for (auto& adjacentPosition : getAdjacentPositions(currentNode.position, map, factionHandler, unit))
		{
			if (adjacentPosition.valid)
			{
				if (m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())].getF() == 0.f)
				{
					float costFromStart = 0.f;
					float costFromEnd = Globals::getDistance(destinationOnGrid, adjacentPosition.position);
					glm::ivec2 cameFrom(0, 0);
					if (isPositionInLineOfSight(currentNode.cameFrom, adjacentPosition.position, map, unit))
					{
						const ThetaStarGraphNode& currentPositionGraphNode = m_thetaGraph[Globals::convertTo1D(currentNode.cameFrom, map.getSize())];
						costFromStart = currentPositionGraphNode.g + Globals::getDistance(adjacentPosition.position, currentNode.cameFrom);
						cameFrom = currentNode.cameFrom;
					}
					else
					{
						costFromStart = currentNode.g + Globals::getDistance(adjacentPosition.position, currentNode.position);
						cameFrom = currentNode.position;
					}

					m_newFrontier.add({ adjacentPosition.position, cameFrom, costFromStart, costFromEnd });
					m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())] =
						{ adjacentPosition.position, cameFrom, costFromStart, costFromEnd };
				}
				else
				{
					ThetaStarGraphNode& adjacentGraphNode = m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())];
					float costFromStart = 0.f;
					glm::ivec2 cameFrom(0, 0);
					float cameFromG = 0.f;
					if (isPositionInLineOfSight(currentNode.cameFrom, adjacentPosition.position, map, unit))
					{
						const ThetaStarGraphNode& camefromGraphNode = m_thetaGraph[Globals::convertTo1D(currentNode.cameFrom, map.getSize())];
						costFromStart = camefromGraphNode.g + Globals::getDistance(adjacentPosition.position, camefromGraphNode.position);
						cameFrom = camefromGraphNode.position;
						cameFromG = camefromGraphNode.g;
					}
					else
					{
						costFromStart = currentNode.g + Globals::getDistance(adjacentPosition.position, currentNode.position);
						cameFrom = currentNode.position;
						cameFromG = currentNode.g;
					}

					if (costFromStart < adjacentGraphNode.g)
					{
						adjacentGraphNode.g = costFromStart;
						adjacentGraphNode.cameFrom = cameFrom;
						if (m_newFrontier.findAndErase(adjacentPosition.position))
						{
							m_newFrontier.add({ adjacentPosition.position,
							cameFrom,
							cameFromG + Globals::getDistance(adjacentPosition.position, cameFrom),
							Globals::getDistance(adjacentPosition.position, destinationOnGrid) });
						}
					}
				}
			}
		}
	}
}

void PathFinding::getPathToPosition(const Worker& worker, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	AdjacentPositions adjacentPositions, const Map& map, const Faction& owningFaction)
{
	if (!getShortestPath(worker, destination, pathToPosition, adjacentPositions, map, owningFaction))
	{
		getGarunteedPath(worker, destination, pathToPosition, adjacentPositions, map, owningFaction);
	}
}

void PathFinding::getPathToPosition(const Worker& worker, const Entity& target, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const Faction& owningFaction)
{
	if (!getShortestPath(worker, target, pathToPosition, map, owningFaction))
	{
		getGarunteedPath(worker, target, pathToPosition, map, owningFaction);
	}
}

bool PathFinding::getShortestPath(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, FactionHandler& factionHandler, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (unit.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return false;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestSqrDistance = Globals::getSqrDistance(destination, unit.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			if (currentNode.position == startingPositionOnGrid)
			{
				if (map.isPositionOnUnitMapAvailable(startingPositionOnGrid, unit.getID()))
				{
					destinationReached = true;
					if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPositionOnGrid));
					}
				}
			}
			else
			{
				destinationReached = true;
				if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
				{
					getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				}
			}
		}
		else
		{
			for (auto& adjacentPosition : getAdjacentPositions(currentNode.position, map, factionHandler, unit))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestSqrDistance)
					{
						shortestSqrDistance = sqrDistance;

						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map, unit))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
								sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
								sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}
	
	return destinationReached;
}

bool PathFinding::getShortestPath(const Worker& worker, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	AdjacentPositions adjacentPositions, const Map& map, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (worker.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return false;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				assert(!pathToPosition.empty());
				*pathToPosition.begin() = destination;
			}
		}
		else
		{
			for (const auto& adjacentPosition : adjacentPositions(currentNode.position))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestDistance)
					{
						shortestDistance = sqrDistance;
						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position), sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position), sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	return destinationReached;
}

bool PathFinding::getShortestPath(const Worker& worker, const Entity& target, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const Faction& owningFaction)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(worker.getPosition(),
		target.getAABB(), map);

	pathToPosition.clear();
	if (worker.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return false;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				assert(!pathToPosition.empty());
				*pathToPosition.begin() = destination;
			}
		}
		else
		{
			for (const auto& adjacentPosition : getAdjacentPositions(currentNode.position, map))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestDistance)
					{
						shortestDistance = sqrDistance;

						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
								sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
								sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	return destinationReached;
}

void PathFinding::getGarunteedPath(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, const Map& map, FactionHandler& factionHandler, 
	const Faction& owningFaction)
{
	pathToPosition.clear();
	if (unit.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestSqrDistance = Globals::getSqrDistance(destination, unit.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			if (currentNode.position == startingPositionOnGrid)
			{
				if (map.isPositionOnUnitMapAvailable(startingPositionOnGrid, unit.getID()))
				{
					destinationReached = true;
					if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPositionOnGrid));
					}
				}
			}
			else
			{
				destinationReached = true;
				if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
				{
					getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				}
			}
		}
		else
		{

			AdjacentPositionsContainer adjacentPositions = getAdjacentPositions(currentNode.position, map, factionHandler, unit);
			for (auto& adjacentPosition : adjacentPositions)
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestSqrDistance)
					{
						adjacentPosition.used = true;
						shortestSqrDistance = sqrDistance;

						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map, unit))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
								sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
								sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
			for (const auto& adjacentPosition : adjacentPositions)
			{
				if (!adjacentPosition.valid || adjacentPosition.used || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
						Globals::convertToWorldPosition(parentNode.position), map, unit))
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
							parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
							sqrDistance);

						addToOpenQueue(m_openQueue, adjacentNode);
					}
					else
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
							currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
							sqrDistance);

						parentNode = currentNode;
						addToOpenQueue(m_openQueue, adjacentNode);
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}
}

void PathFinding::getGarunteedPath(const Worker& worker, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	AdjacentPositions adjacentPositions, const Map& map, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (worker.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				assert(!pathToPosition.empty());
				*pathToPosition.begin() = destination;
			}
		}
		else
		{
			for (auto& adjacentPosition : adjacentPositions(currentNode.position))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestDistance)
					{
						adjacentPosition.used = true;
						shortestDistance = sqrDistance;

						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position), sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position), sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
			for (const auto& adjacentPosition : adjacentPositions(currentNode.position))
			{
				if (!adjacentPosition.valid || adjacentPosition.used || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestDistance)
					{
						shortestDistance = sqrDistance;
						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position), sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position), sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}
}

void PathFinding::getGarunteedPath(const Worker& worker, const Entity& target, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const Faction& owningFaction)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(worker.getPosition(),
		target.getAABB(), map);

	pathToPosition.clear();
	if (worker.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	PriorityQueueNode parentNode = m_openQueue.getTop();
	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				assert(!pathToPosition.empty());
				*pathToPosition.begin() = destination;
			}
		}
		else
		{
			AdjacentPositionsContainer adjacentPositions = getAdjacentPositions(currentNode.position, map);
			for (auto& adjacentPosition : adjacentPositions)
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (sqrDistance < shortestDistance)
					{
						adjacentPosition.used = true;
						shortestDistance = sqrDistance;

						if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
							Globals::convertToWorldPosition(parentNode.position), map))
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
								parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
								sqrDistance);

							addToOpenQueue(m_openQueue, adjacentNode);
						}
						else
						{
							PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
								currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
								sqrDistance);

							parentNode = currentNode;
							addToOpenQueue(m_openQueue, adjacentNode);
						}
					}
				}
			}
			for (const auto& adjacentPosition : adjacentPositions)
			{
				if (!adjacentPosition.valid || adjacentPosition.used || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(destinationOnGrid, adjacentPosition.position);
					if (isPositionInLineOfSight(Globals::convertToWorldPosition(adjacentPosition.position),
						Globals::convertToWorldPosition(parentNode.position), map))
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, parentNode.position,
							parentNode.g + Globals::getSqrDistance(adjacentPosition.position, parentNode.position),
							sqrDistance);

						addToOpenQueue(m_openQueue, adjacentNode);
					}
					else
					{
						PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
							currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
							sqrDistance);

						parentNode = currentNode;
						addToOpenQueue(m_openQueue, adjacentNode);
					}
				}
			}
		}

		m_closedQueue.add(currentNode);

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}
}

