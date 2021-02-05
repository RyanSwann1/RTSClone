#include "Worker.h"
#include "Map.h"
#include "Mineral.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "Faction.h"
#include "SupplyDepot.h"
#include "GameEventHandler.h"
#include "GameEvents.h"
#include "FactionHandler.h"
#include "ShaderHandler.h"
#include "Camera.h"
#ifdef RENDER_PATHING
#include "RenderPathMesh.h"
#endif // RENDER_PATHING

namespace
{
	const float HARVEST_EXPIRATION_TIME = 2.0f;
	const float REPAIR_EXPIRATION_TIME = 3.5f;
	const float BUILD_EXPIRATION_TIME = 2.0f;
	const float MOVEMENT_SPEED = 7.5f;
	const int RESOURCE_CAPACITY = 30;
	const int RESOURCE_INCREMENT = 10;
	const float REPAIR_DISTANCE = static_cast<float>(Globals::NODE_SIZE);
	const float WORKER_PROGRESS_BAR_WIDTH = 60.0f;
	const float WORKER_PROGRESS_BAR_YOFFSET = 30.0f;
}

//BuildingInWorkerQueue
BuildingInWorkerQueue::BuildingInWorkerQueue(const glm::vec3& position, eEntityType entityType)
	: position(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position))),
	entityType(entityType),
	model(ModelManager::getInstance().getModel(entityType))
{}

//Worker
Worker::Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, 
		Globals::WORKER_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
	m_buildQueue(),
	m_repairTargetEntityID(Globals::INVALID_ENTITY_ID),
	m_currentResourceAmount(0),
	m_taskTimer(0.0f, false),
	m_mineralToHarvest(nullptr)
{}

Worker::Worker(Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destination, const Map & map)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, Globals::WORKER_STARTING_HEALTH,
		owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
	m_buildQueue(),
	m_repairTargetEntityID(Globals::INVALID_ENTITY_ID),
	m_currentResourceAmount(0),
	m_taskTimer(0.0f, false),
	m_mineralToHarvest(nullptr)
{
	moveTo(destination, map,
		[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

const Mineral* Worker::getMineralToHarvest() const
{
	return m_mineralToHarvest;
}

const std::list<BuildingInWorkerQueue>& Worker::getBuildingCommands() const
{
	return m_buildQueue;
}

const std::vector<glm::vec3>& Worker::getPathToPosition() const
{
	return m_pathToPosition;
}

eWorkerState Worker::getCurrentState() const
{
	return m_currentState;
}

bool Worker::isHoldingResources() const
{
	return m_currentResourceAmount > 0;
}

int Worker::extractResources()
{
	assert(isHoldingResources());
	int resources = m_currentResourceAmount;
	m_currentResourceAmount = 0;

	return resources;
}

void Worker::setEntityToRepair(const Entity& entity, const Map& map)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, entity.getAABB(), map);
	moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
		eWorkerState::MovingToRepairPosition);

	m_repairTargetEntityID = entity.getID();

	if (m_pathToPosition.empty())
	{
		switchTo(eWorkerState::Repairing);
	}
}

bool Worker::build(const glm::vec3& buildPosition, const Map& map, eEntityType entityType)
{
	if (!map.isPositionOccupied(buildPosition))
	{
		m_buildQueue.emplace_back(buildPosition, entityType);
		if (m_buildQueue.size() == 1)
		{
			moveTo(m_buildQueue.back().position, map,
				[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eWorkerState::MovingToBuildingPosition);
		}

		return true;
	}

	return false;
}

void Worker::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
	Entity::update(deltaTime);

	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		setPosition(newPosition);

		if (m_position == m_pathToPosition.back())
		{
			m_pathToPosition.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eWorkerState::Idle:
		assert(m_pathToPosition.empty());
		break;
	case eWorkerState::Moving:
		if (m_pathToPosition.empty())
		{
			switchTo(eWorkerState::Idle);
		}
		break;
	case eWorkerState::MovingToMinerals:
		if (m_pathToPosition.empty())
		{
			switchTo(eWorkerState::Harvesting);
		}
		break;
	case eWorkerState::ReturningMineralsToHeadquarters:
		assert(isHoldingResources());
		if (m_pathToPosition.empty())
		{
			m_owningFaction.addResources(*this);
			if (m_mineralToHarvest)
			{
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, 
					m_mineralToHarvest->getAABB(), map);
				moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
					eWorkerState::MovingToMinerals, m_mineralToHarvest);
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
		break;
	case eWorkerState::Harvesting:
		assert(m_currentResourceAmount <= RESOURCE_CAPACITY && 
			m_taskTimer.isActive() &&
			m_mineralToHarvest);
		if (m_currentResourceAmount < RESOURCE_CAPACITY)
		{
			if (m_taskTimer.isExpired())
			{
				m_taskTimer.resetElaspedTime();
				m_currentResourceAmount += RESOURCE_INCREMENT;
			}
		}
		else
		{
			const Headquarters& headquarters = m_owningFaction.getClosestHeadquarters(m_position);
			glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, headquarters.getAABB(), map);
			moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
				eWorkerState::ReturningMineralsToHeadquarters);
		}
		break;
	case eWorkerState::MovingToBuildingPosition:
		assert(!m_buildQueue.empty());
		if (m_pathToPosition.empty())
		{
			switchTo(eWorkerState::Building);
		}
		break;
	case eWorkerState::Building:
	{
		assert(m_pathToPosition.empty() && !m_buildQueue.empty() && m_taskTimer.isActive());
		if (m_taskTimer.isExpired())
		{
			const Entity* building = m_owningFaction.spawnBuilding(
				map, m_buildQueue.front().position, m_buildQueue.front().entityType);
			m_buildQueue.pop_front();
			if (!building)
			{
				m_buildQueue.clear();
				switchTo(eWorkerState::Idle);
			}
			else if (!m_buildQueue.empty())
			{
				moveTo(m_buildQueue.front().position, map,
					[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eWorkerState::MovingToBuildingPosition);
			}
			else if(building)
			{
				moveTo(PathFinding::getInstance().getRandomAvailablePositionOutsideAABB(*this, map),
					map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
			}
		}
	}
		break;
	case eWorkerState::MovingToRepairPosition:
		if (m_pathToPosition.empty())
		{
			switchTo(eWorkerState::Repairing);
		}
		break;
	case eWorkerState::Repairing:
		assert(m_pathToPosition.empty() && m_repairTargetEntityID != Globals::INVALID_ENTITY_ID &&
			m_taskTimer.isActive());

		if (m_taskTimer.isExpired())
		{
			m_taskTimer.resetElaspedTime();
			const Entity* targetEntity = m_owningFaction.getEntity(m_repairTargetEntityID);
			if (targetEntity)
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > REPAIR_DISTANCE * REPAIR_DISTANCE)
				{
					setEntityToRepair(*targetEntity, map);
				}
				else if (targetEntity->getHealth() + 1 <= targetEntity->getMaximumHealth())
				{
					m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);

					GameEventHandler::getInstance().gameEvents.push(GameEvent::createRepairEntity(
						m_owningFaction.getController(), m_repairTargetEntityID));
				}
				else
				{
					switchTo(eWorkerState::Idle);
				}
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
		else if (unitStateHandlerTimer.isExpired())
		{
			const Entity* targetEntity = m_owningFaction.getEntity(m_repairTargetEntityID);
			if (targetEntity)
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > REPAIR_DISTANCE * REPAIR_DISTANCE)
				{
					setEntityToRepair(*targetEntity, map);
					break;
				}
				else
				{
					m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
				}
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
		break;
	}

	if (m_taskTimer.isActive())
	{
		m_taskTimer.update(deltaTime);
	}
}

void Worker::moveTo(const glm::vec3& destination, const Map& map, const AdjacentPositions& adjacentPositions, 
	eWorkerState state, const Mineral* mineralToHarvest)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_pathToPosition, m_position);

	PathFinding::getInstance().getPathToPosition(*this, destination, m_pathToPosition, adjacentPositions,
		map, m_owningFaction);
	if (!m_pathToPosition.empty())
	{
		switchTo(state, mineralToHarvest);
	}
	else
	{
		if (previousDestination != m_position)
		{
			m_pathToPosition.push_back(previousDestination);
			switchTo(state);
		}
		else
		{
			switchTo(eWorkerState::Idle);
		}
	}
}

void Worker::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	if (m_currentResourceAmount > 0 && m_currentState != eWorkerState::Harvesting)
	{
		//ModelManager::getInstance().getModel(eModelName::WorkerMineral).render(
			//shaderHandler, { m_position.x - 0.5f, m_position.y, m_position.z - 0.5f });
	}

	Entity::render(shaderHandler, owningFactionController);
}

void Worker::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_taskTimer.isActive())
	{
		float currentTime = m_taskTimer.getElaspedTime() / m_taskTimer.getExpiredTime();
		m_statbarSprite.render(m_position, windowSize, WORKER_PROGRESS_BAR_WIDTH,
			WORKER_PROGRESS_BAR_WIDTH * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT,
			WORKER_PROGRESS_BAR_YOFFSET, shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

void Worker::renderBuildingCommands(ShaderHandler& shaderHandler) const
{
	for (const auto& building : m_buildQueue)
	{
		building.model.get().render(
			shaderHandler, m_owningFaction.getController(), building.position, glm::vec3(0.0f), false);
	}
}

void Worker::renderPathMesh(ShaderHandler& shaderHandler)
{
	RenderPathMesh::render(shaderHandler, m_pathToPosition, m_renderPathMesh);
}

void Worker::switchTo(eWorkerState newState, const Mineral* mineralToHarvest)
{
	//On Exit Current State
	switch (m_currentState)
	{
	case eWorkerState::Idle:
	case eWorkerState::Moving:
		break;
	case eWorkerState::MovingToMinerals:
	case eWorkerState::ReturningMineralsToHeadquarters:
	case eWorkerState::Harvesting:
		if (newState != eWorkerState::MovingToMinerals && 
			newState != eWorkerState::ReturningMineralsToHeadquarters &&
			newState != eWorkerState::Harvesting)
		{
			m_mineralToHarvest = nullptr;
		}
		break;
	case eWorkerState::MovingToBuildingPosition:
	case eWorkerState::Building:
		if (newState != eWorkerState::MovingToBuildingPosition &&
			newState != eWorkerState::Building)
		{
			m_buildQueue.clear();
		}
		break;
	case eWorkerState::MovingToRepairPosition:
	case eWorkerState::Repairing:
		if (newState != eWorkerState::MovingToRepairPosition &&
			newState != eWorkerState::Repairing)
		{
			m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
		}
		break;
	default:
		assert(false);
	}

	//On Enter New State
	switch (newState)
	{
	case eWorkerState::Idle:
		m_taskTimer.setActive(false);
		m_pathToPosition.clear();
		break;
	case eWorkerState::Moving:
	case eWorkerState::MovingToBuildingPosition:
	case eWorkerState::ReturningMineralsToHeadquarters:
	case eWorkerState::MovingToRepairPosition:
		assert(!m_pathToPosition.empty());
		m_taskTimer.setActive(false);
		break;
	case eWorkerState::MovingToMinerals:
		m_taskTimer.setActive(false);
		assert(mineralToHarvest);
		if (mineralToHarvest)
		{
			m_mineralToHarvest = mineralToHarvest;
		}
		break;
	case eWorkerState::Harvesting:
		assert(m_mineralToHarvest);
		m_taskTimer.resetExpirationTime(HARVEST_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_pathToPosition.clear();
		break;
	case eWorkerState::Building:
		m_taskTimer.resetExpirationTime(BUILD_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_pathToPosition.clear();
		break;
	case eWorkerState::Repairing:
		assert(m_repairTargetEntityID != Globals::INVALID_ENTITY_ID);
		m_taskTimer.resetExpirationTime(REPAIR_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_pathToPosition.clear();
		break;
	default:
		assert(false);
	}

	m_taskTimer.resetElaspedTime();
	m_currentState = newState;
}