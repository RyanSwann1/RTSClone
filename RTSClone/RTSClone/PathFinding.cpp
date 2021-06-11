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
	bool isFrontierWithinSizeLimit(const std::queue<glm::ivec2>& frontier, const glm::ivec2& mapSize)
	{
		return static_cast<int>(frontier.size()) <= mapSize.x * mapSize.y;
	}

	bool isPathWithinSizeLimit(const std::vector<glm::vec3>& pathToPosition, const glm::ivec2& mapSize)
	{
		return static_cast<int>(pathToPosition.size()) <= mapSize.x * mapSize.y;
	}

	bool isWithinBuildingPositionsRange(const std::vector<glm::vec3>& buildPositions, const glm::vec3& position)
	{
		auto buildPosition = std::find_if(buildPositions.cbegin(), buildPositions.cend(), [&position](const auto& buildPosition)
		{
			return Globals::getSqrDistance(buildPosition, position) < static_cast<float>(Globals::NODE_SIZE) * 5.0f;
		});

		return buildPosition != buildPositions.cend();
	}

	struct IsInLineOfSightObject
	{
		IsInLineOfSightObject(const glm::vec3& _startingPosition, const glm::vec3& _endingPosition)
			: startingPosition(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(_startingPosition))),
			endingPosition(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(_endingPosition))),
			direction(glm::normalize(endingPosition - startingPosition)),
			distance(glm::distance(endingPosition, startingPosition))
		{}

		IsInLineOfSightObject(glm::ivec2 startingPositionOnGrid, glm::ivec2 targetPositionOnGrid)
			: startingPosition(Globals::convertToWorldPosition(startingPositionOnGrid)),
			endingPosition(Globals::convertToWorldPosition(targetPositionOnGrid)),
			direction(glm::normalize(endingPosition - startingPosition)),
			distance(glm::distance(endingPosition, startingPosition))
		{}

		glm::vec3 startingPosition;
		glm::vec3 endingPosition;
		glm::vec3 direction;
		float distance;
	};
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
	m_BFSGraph(),
	m_BFSFrontier(),
	m_thetaGraph(),
	m_thetaFrontier()
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

bool PathFinding::getClosestAvailablePosition(const Worker& worker, const std::vector<std::unique_ptr<Worker>>& workers, 
	const Map& map, glm::vec3& outPosition)
{
	m_BFSGraph.reset(m_BFSFrontier);
	m_BFSFrontier.emplace(Globals::convertToGridPosition(worker.getPosition()));
	bool availablePositionFound = false;

	int workerID = worker.getID();
	auto workerCollision = [workerID, &workers](glm::ivec2 position) -> bool
	{
		auto worker = std::find_if(workers.cbegin(), workers.cend(), [workerID, position](const auto& worker)
		{
			return worker->getID() != workerID && !worker->getAABB().contains(Globals::convertToWorldPosition(position));
		});
		return worker != workers.cend();
	};

	while (!availablePositionFound && !m_BFSFrontier.empty())
	{
		glm::ivec2 position = m_BFSFrontier.front();
		m_BFSFrontier.pop();

		for (const auto& adjacentPosition : getAdjacentPositions(position, map))// getRandomAdjacentPositions(position, map, unit))
		{
			if (adjacentPosition.valid && workerCollision(adjacentPosition.position))
			{
				outPosition = Globals::convertToWorldPosition(adjacentPosition.position);
				availablePositionFound = true;
				break;
			}
			else if (!m_BFSGraph.isPositionVisited(adjacentPosition.position, map))
			{
				m_BFSGraph.addToGraph(adjacentPosition.position, position, map);
				m_BFSFrontier.push(adjacentPosition.position);
			}
		}

		assert(isFrontierWithinSizeLimit(m_BFSFrontier, map.getSize()));
	}

	return availablePositionFound;
}

bool PathFinding::isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType buildingEntityType, const Map& map, 
	glm::vec3& buildPosition, const FactionAI& owningFaction, const BaseHandler& baseHandler)
{
	m_sharedContainer.clear();
	AABB buildingAABB(startingPosition, ModelManager::getInstance().getModel(buildingEntityType));
	for (int i = 0; i < 5; ++i)
	{
		m_BFSGraph.reset(m_BFSFrontier);
		m_BFSFrontier = std::queue<glm::ivec2>();
		m_BFSFrontier.emplace(Globals::convertToGridPosition(startingPosition));
		bool buildPositionFound = false;
		while (!m_BFSFrontier.empty() && !buildPositionFound)
		{
			glm::ivec2 position = m_BFSFrontier.front();
			m_BFSFrontier.pop();

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
				if (!m_BFSGraph.isPositionVisited(adjacentPosition.position, map))
				{
					m_BFSGraph.addToGraph(adjacentPosition.position, position, map);
					m_BFSFrontier.push(adjacentPosition.position);
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

bool PathFinding::isPositionInLineOfSight(glm::ivec2 startingPositionOnGrid, glm::ivec2 targetPositionOnGrid, const Map& map, const Entity& entity) const
{
	 IsInLineOfSightObject lineOfSightObject(startingPositionOnGrid, targetPositionOnGrid);
	 for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(lineOfSightObject.distance)); i += Globals::NODE_SIZE)
	 {
		 glm::vec3 position = lineOfSightObject.startingPosition+ lineOfSightObject.direction * static_cast<float>(i);
		 if (map.isPositionOccupied(position) ||
			 (entity.getEntityType() == eEntityType::Unit && !map.isPositionOnUnitMapAvailable(Globals::convertToGridPosition(position), entity.getID())))
		 {
			return false;
		 }
	 }

	 return true;
}

bool PathFinding::isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map) const
{
	IsInLineOfSightObject lineOfSightObject(startingPosition, targetEntity.getPosition());
	bool targetEntityVisible = true;
	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(lineOfSightObject.distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = lineOfSightObject.startingPosition+ lineOfSightObject.direction * static_cast<float>(i);
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
	IsInLineOfSightObject lineOfSightObject(startingPosition, targetEntity.getPosition());
	bool targetEntityVisible = true;
	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(lineOfSightObject.distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = lineOfSightObject.startingPosition + lineOfSightObject.direction * static_cast<float>(i);
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
	IsInLineOfSightObject lineOfSightObject(unit.getPosition(), targetEntity.getPosition());
	bool targetEntityVisible = true;
	for (int i = Globals::NODE_SIZE; i <= static_cast<int>(glm::ceil(lineOfSightObject.distance)); i += Globals::NODE_SIZE)
	{
		glm::vec3 position = lineOfSightObject.startingPosition + lineOfSightObject.direction * static_cast<float>(i);
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
	m_BFSGraph.reset(m_BFSFrontier);
	m_BFSFrontier.push(Globals::convertToGridPosition(building.getPosition()));
	bool availablePositionFound = false;

	while (!m_BFSFrontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_BFSFrontier.front();
		m_BFSFrontier.pop();

		for (const auto& adjacentPosition : getAdjacentPositions(position, map, units, workers, building.getAABB()))
		{
			if (adjacentPosition.valid && !building.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition.position)))
			{
				spawnPosition = Globals::convertToWorldPosition(adjacentPosition.position);
				availablePositionFound = true;
				break;
			}
			else if (!m_BFSGraph.isPositionVisited(adjacentPosition.position, map))
			{
				m_BFSGraph.addToGraph(adjacentPosition.position, position, map);
				m_BFSFrontier.push(adjacentPosition.position);
			}	
		}

		assert(isFrontierWithinSizeLimit(m_BFSFrontier, map.getSize()));
	}

	return availablePositionFound;
}

bool PathFinding::getRandomPositionOutsideAABB(const Entity& building, const Map& map, glm::vec3& positionOutsideAABB)
{
	m_BFSGraph.reset(m_BFSFrontier);
	m_BFSFrontier.push(Globals::convertToGridPosition(building.getPosition()));
	bool availablePositionFound = false;

	while (!m_BFSFrontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_BFSFrontier.front();
		m_BFSFrontier.pop();

		for (const auto& adjacentPosition : getRandomAdjacentPositions(position, map, building.getAABB()))
		{
			if (adjacentPosition.valid && !building.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition.position)))
			{
				positionOutsideAABB = Globals::convertToWorldPosition(adjacentPosition.position);
				availablePositionFound = true;
				break;
			}
			else if (!m_BFSGraph.isPositionVisited(adjacentPosition.position, map))
			{
				m_BFSGraph.addToGraph(adjacentPosition.position, position, map);
				m_BFSFrontier.push(adjacentPosition.position);
			}
		}

		assert(isFrontierWithinSizeLimit(m_BFSFrontier, map.getSize()));
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

	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(targetEntity.getPosition());
	std::fill(m_thetaGraph.begin(), m_thetaGraph.end(), ThetaStarGraphNode());
	m_thetaFrontier.clear();
	m_thetaFrontier.add({ startingPositionOnGrid, startingPositionOnGrid, 0.f, Globals::getDistance(destinationOnGrid, startingPositionOnGrid) });
	bool positionFound = false;

	while (!positionFound && !m_thetaFrontier.isEmpty())
	{
		MinHeapNode currentNode = m_thetaFrontier.pop();
		if (Globals::getSqrDistance(targetEntity.getPosition(), Globals::convertToWorldPosition(currentNode.position)) <=
			unit.getAttackRange() * unit.getAttackRange() &&
			isTargetInLineOfSight(unit, targetEntity, map))
		{
			if (currentNode.position == startingPositionOnGrid)
			{
				if (map.isPositionOnUnitMapAvailable(startingPositionOnGrid, unit.getID()))
				{
					positionFound = true;
					if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPositionOnGrid));
					}
				}
			}
			else
			{
				positionFound = true;
				glm::ivec2 position = currentNode.position;
				while (position != startingPositionOnGrid)
				{
					pathToPosition.push_back(Globals::convertToWorldPosition(position));
					position = m_thetaGraph[Globals::convertTo1D(position, map.getSize())].cameFrom;

					assert(isPathWithinSizeLimit(pathToPosition, map.getSize()));
				}
			}
		}
		else
		{
			expandFrontier(currentNode, map, destinationOnGrid, createAdjacentPositions(map, factionHandler, unit), unit);
		}
	}

	return positionFound;
}

void PathFinding::getPathToPosition(const Entity& entity, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, AdjacentPositions adjacentPositions)
{
	pathToPosition.clear();
	if (entity.getPosition() == destination || !map.isWithinBounds(destination))
	{
		return;
	}

	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(entity.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	std::fill(m_thetaGraph.begin(), m_thetaGraph.end(), ThetaStarGraphNode());
	m_thetaFrontier.clear();
	m_thetaFrontier.add({ startingPositionOnGrid, startingPositionOnGrid, 0.f, Globals::getDistance(destinationOnGrid, startingPositionOnGrid) });
	bool destinationReached = false;

	while (!destinationReached && !m_thetaFrontier.isEmpty())
	{
		MinHeapNode currentNode = m_thetaFrontier.pop();
		if (currentNode.position == destinationOnGrid)
		{
			if (currentNode.position == startingPositionOnGrid)
			{
				if (map.isPositionOnUnitMapAvailable(startingPositionOnGrid, entity.getID()))
				{
					destinationReached = true;
					if (Globals::convertToWorldPosition(currentNode.position) != entity.getPosition())
					{
						pathToPosition.push_back(Globals::convertToWorldPosition(startingPositionOnGrid));
					}
				}
			}
			else
			{
				destinationReached = true;
				glm::ivec2 position = currentNode.position;
				while (position != startingPositionOnGrid)
				{
					pathToPosition.push_back(Globals::convertToWorldPosition(position));
					position = m_thetaGraph[Globals::convertTo1D(position, map.getSize())].cameFrom;

					assert(isPathWithinSizeLimit(pathToPosition, map.getSize()));
				}
			}

			if (entity.getEntityType() == eEntityType::Worker)
			{
				if (isPositionInLineOfSight(startingPositionOnGrid, destinationOnGrid, map, entity) && !pathToPosition.empty())
				{
					*pathToPosition.begin() = destination;
				}
			}
		}
		else
		{
			expandFrontier(currentNode, map, destinationOnGrid, adjacentPositions, entity);
		}
	}
}

void PathFinding::expandFrontier(const MinHeapNode& currentNode, const Map& map, glm::ivec2 destinationOnGrid, AdjacentPositions adjacentPositions,
	const Entity& entity)
{
	for (const auto& adjacentPosition : adjacentPositions(currentNode.position))
	{
		if (!adjacentPosition.valid)
		{
			continue;
		}

		if (m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())].getF() == 0.f)
		{
			float costFromStart = 0.f;
			float costFromEnd = Globals::getDistance(destinationOnGrid, adjacentPosition.position);
			glm::ivec2 cameFrom(0, 0);
			if (isPositionInLineOfSight(currentNode.cameFrom, adjacentPosition.position, map, entity))
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

			m_thetaFrontier.add({ adjacentPosition.position, cameFrom, costFromStart, costFromEnd });
			m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())] =
			{ adjacentPosition.position, cameFrom, costFromStart, costFromEnd };
		}
		else
		{
			ThetaStarGraphNode& adjacentGraphNode = m_thetaGraph[Globals::convertTo1D(adjacentPosition.position, map.getSize())];
			float costFromStart = 0.f;
			glm::ivec2 cameFrom(0, 0);
			float cameFromG = 0.f;
			if (isPositionInLineOfSight(currentNode.cameFrom, adjacentPosition.position, map, entity))
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
				if (m_thetaFrontier.findAndErase(adjacentPosition.position))
				{
					m_thetaFrontier.add({ adjacentPosition.position,
					cameFrom,
					cameFromG + Globals::getDistance(adjacentPosition.position, cameFrom),
					adjacentGraphNode.h });
				}
			}
		}
	}
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