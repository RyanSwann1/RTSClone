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
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Base.h"
#ifdef RENDER_PATHING
#include "RenderPrimitiveMesh.h"
#endif // RENDER_PATHING

namespace
{
	constexpr float HARVEST_EXPIRATION_TIME = 2.0f;
	constexpr float REPAIR_EXPIRATION_TIME = 3.5f;
	constexpr float BUILD_EXPIRATION_TIME = 2.0f;
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr int RESOURCE_CAPACITY = 30;
	constexpr int RESOURCE_INCREMENT = 10;
	constexpr int REPAIR_HEALTH_AMOUNT = 1;
	constexpr float REPAIR_DISTANCE = static_cast<float>(Globals::NODE_SIZE) * 3.0f;
	constexpr float WORKER_PROGRESS_BAR_WIDTH = 60.0f;
	constexpr float WORKER_PROGRESS_BAR_YOFFSET = 30.0f;
	constexpr float MAX_DISTANCE_BUILD_HEADQUARTERS = static_cast<float>(Globals::NODE_SIZE) * 12.0f;
}

//BuildingInWorkerQueue
BuildingInWorkerQueue::BuildingInWorkerQueue(const glm::vec3& position, eEntityType entityType)
	: position(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position))),
	entityType(entityType),
	model(ModelManager::getInstance().getModel(entityType))
{}

//Worker
Worker::Worker(Faction& owningFaction, const Map& map, const glm::vec3& startingPosition, const glm::vec3& startingRotation)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, 
		Globals::WORKER_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
	m_buildQueue(),
	m_repairTargetEntity(),
	m_currentResourceAmount(0),
	m_mineralToHarvest(nullptr)
{
	switchTo(eWorkerState::Idle, map);
}

Worker::Worker(Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destination, const Map & map,
	const glm::vec3& startingRotation)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, Globals::WORKER_STARTING_HEALTH,
		owningFaction.getCurrentShieldAmount(), startingRotation),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
	m_buildQueue(),
	m_repairTargetEntity(),
	m_currentResourceAmount(0),
	m_taskTimer(0.0f, false),
	m_mineralToHarvest(nullptr)
{
	moveTo(destination, map);
}

const Mineral* Worker::getMineralToHarvest() const
{
	return m_mineralToHarvest;
}

const std::deque<BuildingInWorkerQueue>& Worker::getBuildingCommands() const
{
	return m_buildQueue;
}

const std::vector<glm::vec3>& Worker::getMovementPath() const
{
	return m_movement.path;
}

eWorkerState Worker::getCurrentState() const
{
	return m_currentState;
}

bool Worker::isHoldingResources() const
{
	return m_currentResourceAmount > 0;
}

bool Worker::isRepairing() const
{
	return m_repairTargetEntity.getID() != Globals::INVALID_ENTITY_ID;
}

bool Worker::isInBuildQueue(eEntityType entityType) const
{
	return std::find_if(m_buildQueue.cbegin(), m_buildQueue.cend(), [entityType](const auto& buildingInQueue)
	{
		return entityType == buildingInQueue.entityType;
	}) != m_buildQueue.cend();
}

int Worker::extractResources()
{
	assert(isHoldingResources());
	int resources = m_currentResourceAmount;
	m_currentResourceAmount = 0;

	return resources;
}

void Worker::add_destination(const glm::vec3& position, const Map& map)
{
	if (m_movement.path.empty())
	{
		moveTo(position, map);
	}
	else
	{
		m_movement.destinations.push(position);
	}
}

void Worker::clear_destinations()
{
	m_movement.destinations = {};
}

void Worker::repairEntity(const Entity& entity, const Map& map)
{
	moveTo(entity, map, eWorkerState::MovingToRepairPosition);
	m_repairTargetEntity.set(entity.getEntityType(), entity.getID());
}

bool Worker::build(const glm::vec3& buildPosition, const Map& map, eEntityType entityType, bool clearBuildQueue)
{	
	if (entityType == eEntityType::Laboratory && 
		(m_owningFaction.get().getLaboratoryCount() == Globals::MAX_LABORATORIES || m_owningFaction.get().isBuildingInAllWorkersQueue(entityType)))
	{
		return false;
	}

	AABB buildingAABB(buildPosition, ModelManager::getInstance().getModel(entityType));
	assert(map.isWithinBounds(buildingAABB) && !map.isAABBOccupied(buildingAABB));

	if (!m_owningFaction.get().isCollidingWithWorkerBuildQueue(ModelManager::getInstance().getModelAABB(buildPosition, entityType)))
	{
		if (clearBuildQueue)
		{
			m_buildQueue.clear();
		}
		if (m_buildQueue.empty())
		{
			moveTo(buildPosition, map, eWorkerState::MovingToBuildingPosition);
			if (m_currentState == eWorkerState::MovingToBuildingPosition)
			{
				m_buildQueue.emplace_back(buildPosition, entityType);
				return true;
			}
		}
		else
		{
			m_buildQueue.emplace_back(buildPosition, entityType);
			return true;
		}
	}

	return false;
}

void Worker::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
	Entity::update(deltaTime);

	if (!m_movement.path.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position, m_movement.path.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position);
		setPosition(newPosition);

		if (m_position == m_movement.path.back())
		{
			m_movement.path.pop_back();
		}
	}

	switch (m_currentState)
	{
	case eWorkerState::Idle:
		assert(m_movement.path.empty());
		break;
	case eWorkerState::Moving:
		if (m_movement.path.empty())
		{
			if (!m_movement.destinations.empty())
			{
				glm::vec3 destination = m_movement.destinations.front();
				m_movement.destinations.pop();
				moveTo(destination, map);
			}
			else
			{
				switchTo(eWorkerState::Idle, map);
			}
		}
		break;
	case eWorkerState::MovingToMinerals:
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Harvesting, map);
		}
		break;
	case eWorkerState::ReturningMineralsToHeadquarters:
		assert(isHoldingResources());
		if (m_movement.path.empty())
		{
			m_owningFaction.get().addResources(*this);
			if (m_mineralToHarvest)
			{
				moveTo(*m_mineralToHarvest, map);
			}
			else
			{
				switchTo(eWorkerState::Idle, map);
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
				int harvestedResource = m_mineralToHarvest->extractQuantity(RESOURCE_INCREMENT);
				if (harvestedResource)
				{
					m_currentResourceAmount += RESOURCE_INCREMENT;
				}
				else
				{
					switchTo(eWorkerState::Idle, map);
				}
			}
		}
		else if(const Headquarters* headquarters = m_owningFaction.get().getClosestHeadquarters(m_position))
		{
			moveTo(*headquarters, map, eWorkerState::ReturningMineralsToHeadquarters);
		}
		break;
	case eWorkerState::MovingToBuildingPosition:
		assert(!m_buildQueue.empty());
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Building, map);
		}
		break;
	case eWorkerState::Building:
		assert(m_movement.path.empty() && !m_buildQueue.empty() && m_taskTimer.isActive());
		if (m_taskTimer.isExpired())
		{
			const Entity* building = m_owningFaction.get().createBuilding(map, *this);
			m_buildQueue.pop_front();
			if (!building)
			{
				m_buildQueue.clear();
				switchTo(eWorkerState::Idle, map);
			}
			else 
			{
				if (!m_buildQueue.empty())
				{
					moveTo(m_buildQueue.front().position, map, building->getAABB(), eWorkerState::MovingToBuildingPosition);
				}
				else
				{
					glm::vec3 destination(0.0f);
					if (PathFinding::getInstance().getRandomPositionOutsideAABB(*building, map, destination))
					{
						moveTo(destination, map, building->getAABB());
					}
					else
					{
						GameEventHandler::getInstance().gameEvents.emplace(
							GameEvent::createForceSelfDestructEntity(m_owningFaction.get().getController(), getID(), getEntityType()));
					}
				}
			}
		}
		break;
	case eWorkerState::MovingToRepairPosition:
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Repairing, map);
		}
		else if (unitStateHandlerTimer.isExpired() &&
			!m_owningFaction.get().getEntity(m_repairTargetEntity.getID(), m_repairTargetEntity.getType()))
		{
			switchTo(eWorkerState::Idle, map);
		}
		break;
	case eWorkerState::Repairing:
		assert(m_movement.path.empty() && m_repairTargetEntity.getID() != Globals::INVALID_ENTITY_ID &&
			m_taskTimer.isActive());

		if (m_taskTimer.isExpired())
		{
			m_taskTimer.resetElaspedTime();
			const Entity* targetEntity = m_owningFaction.get().getEntity(m_repairTargetEntity.getID(), m_repairTargetEntity.getType());
			if (targetEntity && targetEntity->getHealth() < targetEntity->getMaximumHealth())
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > REPAIR_DISTANCE * REPAIR_DISTANCE)
				{
					repairEntity(*targetEntity, map);
				}
				else
				{
					m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);

					GameEventHandler::getInstance().gameEvents.push(GameEvent::createRepairEntity(
						m_owningFaction.get().getController(), m_repairTargetEntity.getID(), m_repairTargetEntity.getType()));
				}
			}
			else
			{
				switchTo(eWorkerState::Idle, map);
			}
		}
		else if (unitStateHandlerTimer.isExpired())
		{
			const Entity* targetEntity = m_owningFaction.get().getEntity(m_repairTargetEntity.getID(), m_repairTargetEntity.getType());
			if (targetEntity && targetEntity->getHealth() < targetEntity->getMaximumHealth())
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position) > REPAIR_DISTANCE * REPAIR_DISTANCE)
				{
					repairEntity(*targetEntity, map);
				}
				else
				{
					m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
				}
			}
			else
			{
				switchTo(eWorkerState::Idle, map);
			}
		}
		break;
	}

	if (m_taskTimer.isActive())
	{
		m_taskTimer.update(deltaTime);
	}
}

void Worker::moveTo(const Mineral& mineral, const Map& map)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, mineral.getAABB(), map);

	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map));
	if (!m_movement.path.empty())
	{
		switchTo(eWorkerState::MovingToMinerals, map, &mineral);
	}
	else
	{
		if (previousDestination != m_position)
		{
			m_movement.path.push_back(previousDestination);
			switchTo(eWorkerState::Moving, map);
		}
		else
		{
			switchTo(eWorkerState::Idle, map);
		}
	}
}

void Worker::moveTo(const Entity& target, const Map& map, eWorkerState state)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(getPosition(), target.getAABB(), map);
	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map));
	if (!m_movement.path.empty())
	{
		switchTo(state, map);
	}
	else
	{
		if (previousDestination != m_position)
		{
			m_movement.path.push_back(previousDestination);
			switchTo(state, map);
		}
		else
		{
			switchTo(eWorkerState::Idle, map);
		}
	}
}

void Worker::moveTo(const glm::vec3& destination, const Map& map, eWorkerState state /*= eWorkerState::Moving*/)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);
	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map));
	if (!m_movement.path.empty())
	{
		switchTo(state, map);
	}
	else
	{
		if (previousDestination != m_position)
		{
			m_movement.path.push_back(previousDestination);
			switchTo(state, map);
		}
		else
		{
			switchTo(eWorkerState::Idle, map);
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
			shaderHandler, m_owningFaction.get().getController(), building.position, glm::vec3(0.0f), false);
	}
}

#ifdef RENDER_PATHING
void Worker::render_path(ShaderHandler& shaderHandler)
{
	RenderPrimitiveMesh::render(shaderHandler, m_movement.path, m_movement.pathMesh);
}
#endif //RENDER_PATHING

void Worker::switchTo(eWorkerState newState, const Map& map, const Mineral* mineralToHarvest)
{
	//On Exit Current State
	switch (m_currentState)
	{
	case eWorkerState::Idle:
	break;
	case eWorkerState::Moving:
		if (m_currentState != newState) 
		{	
			m_movement.destinations = {};
		}
	break;
	case eWorkerState::MovingToMinerals:
	case eWorkerState::ReturningMineralsToHeadquarters:
		if (newState != eWorkerState::MovingToMinerals &&
			newState != eWorkerState::ReturningMineralsToHeadquarters &&
			newState != eWorkerState::Harvesting)
		{
			m_mineralToHarvest = nullptr;
		}
		break;
	case eWorkerState::Harvesting:
		if (newState != eWorkerState::MovingToMinerals && 
			newState != eWorkerState::ReturningMineralsToHeadquarters &&
			newState != eWorkerState::Harvesting)
		{
			m_mineralToHarvest = nullptr;
			m_taskTimer.setActive(false);
			m_taskTimer.resetElaspedTime();
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
		if (newState != eWorkerState::MovingToRepairPosition &&
			newState != eWorkerState::Repairing)
		{
			m_repairTargetEntity.reset();
		}
		break;
	case eWorkerState::Repairing:
		if (newState != eWorkerState::MovingToRepairPosition &&
			newState != eWorkerState::Repairing)
		{
			m_repairTargetEntity.reset();
			m_taskTimer.setActive(false);
			m_taskTimer.resetElaspedTime();
		}
		break;
	default:
		assert(false);
	}

	
	//On Enter New State
	eWorkerState oldState = m_currentState;
	m_currentState = newState;
	switch (newState)
	{
	case eWorkerState::Idle:
		assert(m_buildQueue.empty());
		m_taskTimer.setActive(false);
		m_movement.path.clear();
		if (oldState != eWorkerState::Idle)
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::create_entity_idle(getID(), m_owningFaction.get().getController()));
		}
		break;
	case eWorkerState::Moving:
	case eWorkerState::ReturningMineralsToHeadquarters:
	case eWorkerState::MovingToBuildingPosition:
		m_taskTimer.setActive(false);
		break;
	case eWorkerState::MovingToRepairPosition:
		assert(!m_movement.path.empty());
		m_taskTimer.setActive(false);
		m_taskTimer.resetElaspedTime();
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
		m_movement.path.clear();
		break;
	case eWorkerState::Building:
		m_taskTimer.resetExpirationTime(BUILD_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_movement.path.clear();
		break;
	case eWorkerState::Repairing:
		assert(m_repairTargetEntity.getID() != Globals::INVALID_ENTITY_ID);
		m_taskTimer.resetExpirationTime(REPAIR_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_movement.path.clear();
		break;
	default:
		assert(false);
	}

	m_taskTimer.resetElaspedTime();
	
}

void Worker::moveTo(const glm::vec3& destination, const Map& map, const AABB& ignoreAABB, eWorkerState state)
{
	assert(m_currentState == eWorkerState::Building);
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position);

	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map, ignoreAABB));
	if (!m_movement.path.empty())
	{
		switchTo(state, map);
	}
	else
	{
		if (previousDestination != m_position)
		{
			m_movement.path.push_back(previousDestination);
			switchTo(state, map);
		}
		else
		{
			switchTo(eWorkerState::Idle, map);
		}
	}
}