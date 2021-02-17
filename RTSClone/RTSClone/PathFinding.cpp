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

	void convertPathToWaypoints(std::vector<glm::vec3>& pathToPosition, const Entity& entity, const Map& map)
	{
		if (pathToPosition.size() <= size_t(1))
		{
			return;
		}

		std::queue<glm::vec3> positionsToKeep;
		int positionIndex = 0;
		glm::vec3 startingPosition = entity.getPosition();
		while (startingPosition != pathToPosition.front() &&
			positionIndex < pathToPosition.size())
		{
			glm::vec3 targetPosition = pathToPosition[positionIndex];
			glm::vec3 position = startingPosition;
			float distance = glm::distance(targetPosition, startingPosition);
			constexpr float step = Globals::NODE_SIZE;
			bool collision = false;

			for (int ray = step; ray <= static_cast<int>(distance); ray += step)
			{
				position = position + glm::normalize(targetPosition - startingPosition) * step;
				if (map.isPositionOccupied(position))
				{
					collision = true;
					break;
				}
			}

			if (!collision)
			{
				positionsToKeep.push(pathToPosition[positionIndex]);
				startingPosition = pathToPosition[positionIndex];
				positionIndex = 0;

				//TODO: Due to duplications - need to investigate
				if (positionsToKeep.size() > pathToPosition.size())
				{
					return;
				}
			}
			else
			{
				++positionIndex;
			}
		}

		if (!positionsToKeep.empty())
		{
			pathToPosition.clear();
			while (!positionsToKeep.empty())
			{
				const glm::vec3& positionToKeep = positionsToKeep.front();
				pathToPosition.push_back(positionToKeep);
				positionsToKeep.pop();
			}

			std::reverse(pathToPosition.begin(), pathToPosition.end());
		}
	}

	void convertPathToWaypoints(std::vector<glm::vec3>& pathToPosition, const Entity& entity, FactionHandler& factionHandler,
		const Map& map, const Faction& owningFaction)
	{
		if (pathToPosition.size() <= size_t(1))
		{
			return;
		}

		std::queue<glm::vec3> positionsToKeep;
		int positionIndex = 0;
		glm::vec3 startingPosition = entity.getPosition();
		while (startingPosition != pathToPosition.front() && 
			positionIndex < pathToPosition.size())
		{
			glm::vec3 targetPosition = pathToPosition[positionIndex];
			glm::vec3 position = startingPosition;
			float distance = glm::distance(targetPosition, startingPosition);
			constexpr float step = Globals::NODE_SIZE;
			bool collision = false;

			for (int ray = step; ray <= static_cast<int>(distance); ray += step)
			{
				position = position + glm::normalize(targetPosition - startingPosition) * step;

				if (!PathFinding::getInstance().isUnitPositionAvailable(position, entity, factionHandler, owningFaction) || map.isPositionOccupied(position))
				{
					collision = true;
					break;
				}
			}

			if (!collision)
			{
				positionsToKeep.push(pathToPosition[positionIndex]);
				startingPosition = pathToPosition[positionIndex];
				positionIndex = 0;

				//TODO: Due to duplications - need to investigate
				if (positionsToKeep.size() > pathToPosition.size())
				{
					return;
				}
			}
			else
			{
				++positionIndex;
			}
		}

		if (!positionsToKeep.empty())
		{
			pathToPosition.clear();
			while (!positionsToKeep.empty())
			{
				const glm::vec3& positionToKeep = positionsToKeep.front();
				pathToPosition.push_back(positionToKeep);
				positionsToKeep.pop();
			}

			std::reverse(pathToPosition.begin(), pathToPosition.end());
		}
	}
}

PathFinding::PathFinding()
	: m_sharedPositionContainer(),
	m_graph(),
	m_frontier(),
	m_openQueue(),
	m_closedQueue()
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
	m_sharedPositionContainer.clear();
	m_sharedPositionContainer.reserve(
		static_cast<size_t>(gameMessage.mapSize.x) * static_cast<size_t>(gameMessage.mapSize.y));
}

bool PathFinding::isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType buildingEntityType, const Map& map, 
	glm::vec3& buildPosition, const FactionAI& owningFaction, const BaseHandler& baseHandler)
{
	std::vector<glm::vec3> buildPositions;
	AABB buildingAABB(startingPosition, ModelManager::getInstance().getModel(buildingEntityType));
	for (int i = 0; i < 15; ++i)
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
						!owningFaction.isWithinDistanceOfBuildings(Globals::convertToWorldPosition(adjacentPosition.position), Globals::NODE_SIZE * 3.0f) && 
						!baseHandler.isWithinDistanceOfMinerals(Globals::convertToWorldPosition(adjacentPosition.position), Globals::NODE_SIZE * 6.0f))
					{
						buildPositionFound = true;
						buildPositions.emplace_back(Globals::convertToWorldPosition(adjacentPosition.position));
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

	if (!buildPositions.empty())
	{
		buildPosition = buildPositions[Globals::getRandomNumber(0, static_cast<int>(buildPositions.size()) - 1)];
		return true;
	}

	return false;
}

bool PathFinding::isUnitPositionAvailable(const glm::vec3& position, const Entity& entity, FactionHandler& factionHandler, 
	const Faction& owningFaction) const
{
	for (const auto& opposingFaction : factionHandler.getOpposingFactions(owningFaction.getController()))
	{
		auto unit = std::find_if(opposingFaction.get().getUnits().cbegin(), opposingFaction.get().getUnits().cend(), [&position](const auto& unit)
		{
			if (!unit.getPathToPosition().empty())
			{
				return unit.getPathToPosition().front() == position;
			}
			else
			{
				return unit.getAABB().contains(position);
			}
		});
		if (unit != opposingFaction.get().getUnits().cend())
		{
			return false;
		}
	}

	assert(factionHandler.isFactionActive(owningFaction.getController()));
	int senderUnitID = entity.getID();
	auto unit = std::find_if(owningFaction.getUnits().cbegin(), owningFaction.getUnits().cend(), [&position, senderUnitID](const auto& unit)
	{
		if (!unit.getPathToPosition().empty())
		{
			return unit.getID() != senderUnitID && unit.getPathToPosition().front() == position;
		}
		else
		{
			return unit.getID() != senderUnitID && unit.getAABB().contains(position);
		}
	});
	if (unit != owningFaction.getUnits().cend())
	{
		return false;
	}

	return true;
}

bool PathFinding::isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map) const
{
	glm::vec3 direction = glm::normalize(targetEntity.getPosition() - startingPosition);
	constexpr float step = 0.5f;
	float distanceSqr = Globals::getSqrDistance(targetEntity.getPosition(), startingPosition); 
	bool targetEntityVisible = true;

	for (int i = 1; i < std::ceil(distanceSqr / (step * step)); ++i)
	{
		glm::vec3 position = startingPosition + direction * static_cast<float>(i);
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
	glm::vec3 direction = glm::normalize(targetEntity.getPosition() - startingPosition);
	constexpr float step = 0.5f;
	float distanceSqr = Globals::getSqrDistance(targetEntity.getPosition(), startingPosition);
	bool targetEntityVisible = true;

	for (int i = 1; i < std::ceil(distanceSqr / step * step); ++i)
	{
		glm::vec3 position = startingPosition + direction * static_cast<float>(i);
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

const std::vector<glm::vec3>& PathFinding::getFormationPositions(const glm::vec3& startingPosition,
	const std::vector<Unit*>& selectedUnits, const Map& map)
{
	//TODO: Sort by closest
	assert(!selectedUnits.empty() && std::find(selectedUnits.cbegin(), selectedUnits.cend(), nullptr) == selectedUnits.cend());
	m_graph.reset(m_frontier);
	m_sharedPositionContainer.clear();

	int selectedUnitIndex = 0;
	m_frontier.push(Globals::convertToGridPosition(startingPosition));

	while (!m_frontier.empty() && m_sharedPositionContainer.size() < selectedUnits.size())
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions = getAdjacentPositions(position, map);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid)
			{
				if (!m_graph.isPositionVisited(adjacentPosition.position, map))
				{
					m_graph.addToGraph(adjacentPosition.position, position, map);
					m_frontier.push(adjacentPosition.position);

					assert(selectedUnitIndex < selectedUnits.size());
					m_sharedPositionContainer.emplace_back(Globals::convertToWorldPosition(adjacentPosition.position));
					++selectedUnitIndex;
					
					if (m_sharedPositionContainer.size() == selectedUnits.size())
					{
						break;
					}
				}
			}
		}
	}

	return m_sharedPositionContainer;
}

glm::vec3 PathFinding::getClosestAvailablePosition(const glm::vec3& startingPosition, const std::list<Unit>& units, 
	const std::list<Worker>& workers, const Map& map)
{
	m_graph.reset(m_frontier);
	m_frontier.push(Globals::convertToGridPosition(startingPosition));
	glm::ivec2 availablePositionOnGrid = {0, 0};
	bool availablePositionFound = false;

	while (!m_frontier.empty() && !availablePositionFound)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		for (const auto& adjacentPosition : getAdjacentPositions(position, map, units, workers))
		{
			if (adjacentPosition.valid)
			{
				availablePositionOnGrid = adjacentPosition.position;
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

	return Globals::convertToWorldPosition(availablePositionOnGrid);
}

glm::vec3 PathFinding::getRandomAvailablePositionOutsideAABB(const Entity& senderEntity, const Map& map)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::array<glm::ivec2, 8> shuffledAllDirectionsOnGrid = ALL_DIRECTIONS_ON_GRID;
	std::shuffle(shuffledAllDirectionsOnGrid.begin(), shuffledAllDirectionsOnGrid.end(), g);

	for (glm::vec2 direction : shuffledAllDirectionsOnGrid)
	{
		glm::vec2 position = Globals::convertToGridPosition(senderEntity.getPosition());
		for (int i = 1; i <= Globals::NODE_SIZE * 10; ++i)
		{
			position = position + direction * static_cast<float>(i);
			if (!map.isPositionOccupied(position) && map.isWithinBounds(position))
			{
				return Globals::convertToWorldPosition(position);
			}
		}
	}

	return senderEntity.getPosition();
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

	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 targetPositionOnGrid = Globals::convertToGridPosition(targetEntity.getPosition());
	bool positionFound = false;

	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(targetEntity.getPosition(), unit.getPosition()) });

	while (!positionFound && !m_openQueue.isEmpty())
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (Globals::getSqrDistance(targetEntity.getPosition(), Globals::convertToWorldPosition(currentNode.position)) <=
			unit.getAttackRange() * unit.getAttackRange() && 
			isTargetInLineOfSight(Globals::convertToWorldPosition(currentNode.position), targetEntity, map))
		{
			if (currentNode.position == startingPositionOnGrid)
			{
				if (isUnitPositionAvailable(Globals::convertToWorldPosition(startingPositionOnGrid), unit, factionHandler, unit.getOwningFaction()))
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
				if (Globals::convertToWorldPosition(currentNode.position) != unit.getPosition())
				{
					getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
				}
			}
		}
		else
		{
			for (const auto& adjacentPosition : getAdjacentPositions(currentNode.position, map, factionHandler, unit))
			{
				if (!adjacentPosition.valid || m_closedQueue.contains(adjacentPosition.position))
				{
					continue;
				}
				else
				{
					float sqrDistance = Globals::getSqrDistance(targetPositionOnGrid, adjacentPosition.position);
					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
						sqrDistance);

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

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	convertPathToWaypoints(pathToPosition, unit, factionHandler, map, unit.getOwningFaction());
	
	return positionFound;
}

void PathFinding::getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, FactionHandler& factionHandler, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (unit.getPosition() == destination)
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(unit.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, unit.getPosition());
	glm::ivec2 closestAvailablePosition = { 0, 0 };
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f, 
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			switch (unit.getEntityType())
			{
			case eEntityType::Unit:
				break;
			case eEntityType::Worker:
				pathToPosition.push_back(destination);
				break;
			default:
				assert(false);
			}

			if (currentNode.position == startingPositionOnGrid)
			{
				if (isUnitPositionAvailable(Globals::convertToWorldPosition(startingPositionOnGrid), unit, factionHandler, owningFaction))
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
			for (const auto& adjacentPosition : getAdjacentPositions(currentNode.position, map, factionHandler, unit))
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
						closestAvailablePosition = adjacentPosition.position;
						shortestDistance = sqrDistance;
					}
					
					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
						sqrDistance);

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

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) && 
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	if (pathToPosition.empty() && shortestDistance != Globals::getSqrDistance(destination, unit.getPosition()))
	{	
		getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, closestAvailablePosition, m_closedQueue, map);
	}

	convertPathToWaypoints(pathToPosition, unit, factionHandler, map, owningFaction);
}

void PathFinding::getPathToPosition(const Worker& worker, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, 
	AdjacentPositions adjacentPositions, const Map& map, const Faction& owningFaction)
{
	pathToPosition.clear();
	if (worker.getPosition() == destination)
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	glm::ivec2 closestAvailablePosition = { 0, 0 };
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			switch (worker.getEntityType())
			{
			case eEntityType::Unit:
				break;
			case eEntityType::Worker:
				pathToPosition.push_back(destination);
				break;
			default:
				assert(false);
			}

			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
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
						closestAvailablePosition = adjacentPosition.position;
						shortestDistance = sqrDistance;
					}

					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
						sqrDistance);

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

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	if (pathToPosition.empty() && shortestDistance != Globals::getSqrDistance(destination, worker.getPosition()))
	{
		getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, closestAvailablePosition, m_closedQueue, map);
	}

	convertPathToWaypoints(pathToPosition, worker, map);
}

void PathFinding::getPathToPosition(const Worker& worker, const Entity& target, std::vector<glm::vec3>& pathToPosition, 
	const Map& map, const Faction& owningFaction)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(worker.getPosition(),
		target.getAABB(), map);

	pathToPosition.clear();
	if (worker.getPosition() == destination)
	{
		return;
	}

	m_openQueue.clear();
	m_closedQueue.clear();

	bool destinationReached = false;
	glm::ivec2 startingPositionOnGrid = Globals::convertToGridPosition(worker.getPosition());
	glm::ivec2 destinationOnGrid = Globals::convertToGridPosition(destination);
	float shortestDistance = Globals::getSqrDistance(destination, worker.getPosition());
	glm::ivec2 closestAvailablePosition = { 0, 0 };
	m_openQueue.add({ startingPositionOnGrid, startingPositionOnGrid, 0.0f,
		Globals::getSqrDistance(destinationOnGrid, startingPositionOnGrid) });

	while (!m_openQueue.isEmpty() && !destinationReached)
	{
		PriorityQueueNode currentNode = m_openQueue.getTop();
		m_openQueue.popTop();

		if (currentNode.position == destinationOnGrid)
		{
			switch (worker.getEntityType())
			{
			case eEntityType::Unit:
				break;
			case eEntityType::Worker:
				pathToPosition.push_back(destination);
				break;
			default:
				assert(false);
			}

			destinationReached = true;
			if (Globals::convertToWorldPosition(currentNode.position) != worker.getPosition())
			{
				getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, currentNode, m_closedQueue, map);
			}
		}
		else
		{
			for (const auto& adjacentPosition : getAdjacentPositions(currentNode.position, map))// adjacentPositions(currentNode.position))
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
						closestAvailablePosition = adjacentPosition.position;
						shortestDistance = sqrDistance;
					}

					PriorityQueueNode adjacentNode(adjacentPosition.position, currentNode.position,
						currentNode.g + Globals::getSqrDistance(adjacentPosition.position, currentNode.position),
						sqrDistance);

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

		assert(isPriorityQueueWithinSizeLimit(m_openQueue, map.getSize()) &&
			isPriorityQueueWithinSizeLimit(m_closedQueue, map.getSize()));
	}

	if (pathToPosition.empty() && shortestDistance != Globals::getSqrDistance(destination, worker.getPosition()))
	{
		getPathFromClosedQueue(pathToPosition, startingPositionOnGrid, closestAvailablePosition, m_closedQueue, map);
	}

	convertPathToWaypoints(pathToPosition, worker, map);
}
