#include "Worker.h"
#include "Core/Map.h"
#include "Core/Mineral.h"
#include "Graphics/ModelManager.h"
#include "Core/PathFinding.h"
#include "Factions/Faction.h"
#include "SupplyDepot.h"
#include "Events/GameEvents.h"
#include "Factions/FactionHandler.h"
#include "Graphics/ShaderHandler.h"
#include "Core/Camera.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Core/Base.h"
#include "Core/Level.h"
#ifdef RENDER_PATHING
#include "Graphics/RenderPrimitiveMesh.h"
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
WorkerScheduledBuilding::WorkerScheduledBuilding(const glm::vec3& position, eEntityType entityType)
	: position(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position))),
	entityType(entityType),
	model(ModelManager::getInstance().getModel(entityType))
{}

//Worker
Worker::Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, 
		Globals::WORKER_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), false, startingRotation),
	m_owningFaction(&owningFaction)
{
	switchTo(eWorkerState::Idle);
}

Worker::Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation,
	const glm::vec3& destination, const Map& map)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, 
		Globals::WORKER_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), false, startingRotation),
	m_owningFaction(&owningFaction)
{
	MoveTo(destination, map, false);
}

const Mineral* Worker::getMineralToHarvest() const
{
	return m_mineralToHarvest;
}

const std::deque<WorkerScheduledBuilding>& Worker::get_scheduled_buildings() const
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

bool Worker::is_colliding_with_scheduled_buildings(const AABB& aabb) const
{
	return std::any_of(m_buildQueue.cbegin(), m_buildQueue.cend(), [&aabb](auto& buildingCommand)
	{
		return aabb.contains(buildingCommand.position);
	});
}

bool Worker::isHoldingResources() const
{
	return m_resources.has_value();
}

bool Worker::isRepairing() const
{
	return m_repairTargetEntity.has_value();
}

bool Worker::isInBuildQueue(eEntityType entityType) const
{
	return std::find_if(m_buildQueue.cbegin(), m_buildQueue.cend(), [entityType](const auto& buildingInQueue)
	{
		return entityType == buildingInQueue.entityType;
	}) != m_buildQueue.cend();
}

bool Worker::is_group_selectable() const
{
	return true;
}

int Worker::extractResources()
{
	assert(isHoldingResources());
	if (m_resources)
	{
		const int resources = *m_resources;
		m_resources = std::nullopt;
		return resources;
	}
	return 0;
}

void Worker::clear_destinations()
{
	m_movement.destinations = {};
}

bool Worker::repairEntity(const Entity& entity, const Map& map)
{
	if (!Entity::repairEntity(entity, map))
	{
		return false;
	}

	m_repairTargetEntity = { entity.getID() };
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(entity.getPosition(), entity.getAABB(), map);
	move_to(destination, map, eWorkerState::MovingToRepairPosition);	
	
	return true;
}

bool Worker::build(const Faction& owningFaction, const glm::vec3& buildPosition, const Map& map, eEntityType entityType, bool clearBuildQueue)
{	
	if (entityType == eEntityType::Laboratory && 
		(owningFaction.is_laboratory_built() || owningFaction.isBuildingInAllWorkersQueue(entityType)))
	{
		return false;
	}

	AABB buildingAABB(buildPosition, ModelManager::getInstance().getModel(entityType));
	assert(map.isWithinBounds(buildingAABB) && !map.isAABBOccupied(buildingAABB));

	if (!owningFaction.isCollidingWithWorkerBuildQueue(ModelManager::getInstance().getModelAABB(buildPosition, entityType)))
	{
		if (clearBuildQueue)
		{
			m_buildQueue.clear();
		}
		if (m_buildQueue.empty())
		{
			move_to(buildPosition, map, eWorkerState::MovingToBuildingPosition);
		}
		m_buildQueue.emplace_back(buildPosition, entityType);
		if (m_currentState == eWorkerState::Idle)
		{
			switchTo(eWorkerState::Building);
		}
		return true;
	}

	return false;
}

void Worker::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
	Entity::update(deltaTime);

	if (!m_movement.path.empty())
	{
		glm::vec3 newPosition = Globals::moveTowards(m_position.Get(), m_movement.path.back(), MOVEMENT_SPEED * deltaTime);
		m_rotation.y = Globals::getAngle(newPosition, m_position.Get());
		setPosition(newPosition);

		if (m_position.Get() == m_movement.path.back())
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
				MoveTo(destination, map, false);
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
		break;
	case eWorkerState::MovingToMinerals:
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Harvesting);
		}
		break;
	case eWorkerState::ReturningMineralsToHeadquarters:
		assert(isHoldingResources());
		if (m_movement.path.empty())
		{
			Level::add_event(GameEvent::create<AddFactionResourcesEvent>({ *m_resources, m_owningFaction->getController() }));
			m_resources = std::nullopt;
			if (m_mineralToHarvest)
			{
				Harvest(*m_mineralToHarvest, map);
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
		break;
	case eWorkerState::Harvesting:
	{
		assert(m_resources <= RESOURCE_CAPACITY &&
			m_taskTimer.isActive() &&
			m_mineralToHarvest);
		if (m_resources < RESOURCE_CAPACITY)
		{
			if (m_taskTimer.isExpired())
			{
				m_taskTimer.resetElaspedTime();
				int harvestedResource = m_mineralToHarvest->extractQuantity(RESOURCE_INCREMENT);
				if (harvestedResource)
				{
					if (!m_resources)
					{
						m_resources.emplace(RESOURCE_INCREMENT);
					}
					else
					{
						*m_resources += RESOURCE_INCREMENT;
					}
				}
				else
				{
					switchTo(eWorkerState::Idle);
				}
			}
		}
		else if (const Headquarters* headquarters = m_owningFaction->get_closest_headquarters(m_position.Get()))
		{
			return_minerals_to_headquarters(*headquarters, map);
		}
	}
		break;
	case eWorkerState::MovingToBuildingPosition:
		assert(!m_buildQueue.empty());
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Building);
		}
		break;
	case eWorkerState::Building:
	{
		assert(m_movement.path.empty() && !m_buildQueue.empty() && m_taskTimer.isActive());
		if (m_taskTimer.isExpired())
		{
			const Entity* building = m_owningFaction->create_building(*this, map);
			m_buildQueue.pop_front();
			if (!building)
			{
				m_buildQueue.clear();
				switchTo(eWorkerState::Idle);
			}
			else
			{
				if (!m_buildQueue.empty())
				{
					move_to(m_buildQueue.front().position, map, building->getAABB(), eWorkerState::MovingToBuildingPosition);
				}
				else
				{
					glm::vec3 destination(0.0f);
					if (PathFinding::getInstance().getRandomPositionOutsideAABB(*building, map, destination))
					{
						move_to(destination, map, building->getAABB(), eWorkerState::Moving);
					}
					else
					{
						Level::add_event(GameEvent::create<SelfDestructEntityEvent>({ m_owningFaction->getController(), getID(), getEntityType() }));
					}
				}
			}
		}
	}

		break;
	case eWorkerState::MovingToRepairPosition:
	{
		if (m_movement.path.empty())
		{
			switchTo(eWorkerState::Repairing);
		}
	}
		break;
	case eWorkerState::Repairing:
	{
		assert(m_movement.path.empty() &&
			m_repairTargetEntity &&
			m_taskTimer.isActive());
		if (m_taskTimer.isExpired())
		{
			m_taskTimer.resetElaspedTime();
			const Entity* targetEntity = m_owningFaction->get_entity(*m_repairTargetEntity);
			if (targetEntity && targetEntity->getHealth() < targetEntity->getMaximumHealth())
			{
				if (Globals::getSqrDistance(targetEntity->getPosition(), m_position.Get()) > REPAIR_DISTANCE * REPAIR_DISTANCE)
				{
					repairEntity(*targetEntity, map);
				}
				else
				{
					m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position.Get());

					Level::add_event(GameEvent::create<RepairEntityEvent>({ m_owningFaction->getController(), *m_repairTargetEntity }));
				}
			}
			else
			{
				switchTo(eWorkerState::Idle);
			}
		}
	}
	break;
	}

	if (m_taskTimer.isActive())
	{
		m_taskTimer.update(deltaTime);
	}
}

void Worker::delayed_update(const Map& map, FactionHandler& factionHandler)
{
	switch (m_currentState)
	{
	
	case eWorkerState::MovingToRepairPosition:
	{
		if (!m_owningFaction->get_entity(*m_repairTargetEntity))
		{
			switchTo(eWorkerState::Idle);
		}
	}
	break;
	case eWorkerState::Repairing:
	{
		assert(m_movement.path.empty() &&
			m_repairTargetEntity &&
			m_taskTimer.isActive());

		const Entity* targetEntity = m_owningFaction->get_entity(*m_repairTargetEntity);
		if (targetEntity && targetEntity->getHealth() < targetEntity->getMaximumHealth())
		{
			if (Globals::getSqrDistance(targetEntity->getPosition(), m_position.Get()) > REPAIR_DISTANCE * REPAIR_DISTANCE)
			{
				repairEntity(*targetEntity, map);
			}
			else
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position.Get());
			}
		}
		else
		{
			switchTo(eWorkerState::Idle);
		}
	}
	break;
	}
}

bool Worker::Harvest(const Mineral& mineral, const Map& map)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position.Get(), mineral.getAABB(), map);
	m_mineralToHarvest = &mineral;
	move_to(destination, map, eWorkerState::MovingToMinerals);
	return true;
}

void Worker::return_minerals_to_headquarters(const Headquarters& headquarters, const Map& map)
{
	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(getPosition(), headquarters.getAABB(), map);
	move_to(destination, map, eWorkerState::ReturningMineralsToHeadquarters);
}

bool Worker::MoveTo(const glm::vec3& position, const Map& map, const bool add_to_destinations)
{
	if (m_movement.IsMovableAfterAddingDestination(add_to_destinations, position))
	{
		return move_to(position, map, eWorkerState::Moving);
	}

	return false;
}

void Worker::revalidate_movement_path(const Map& map)
{
	if (!m_movement.path.empty())
	{
		switch (m_currentState)
		{
		case eWorkerState::Moving:
		case eWorkerState::ReturningMineralsToHeadquarters:
		case eWorkerState::MovingToBuildingPosition:
		case eWorkerState::MovingToRepairPosition:
			move_to(m_movement.path.front(), map, m_currentState);
			break;
		case eWorkerState::MovingToMinerals:
			assert(m_mineralToHarvest);
			Harvest(*m_mineralToHarvest, map);
			break;
		case eWorkerState::Idle:
		case eWorkerState::Harvesting:
		case eWorkerState::Building:
		case eWorkerState::Repairing:
			break;
		default:
			assert(false);
		}
	}
}

void Worker::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	if (m_resources && m_currentState != eWorkerState::Harvesting)
	{
		//ModelManager::getInstance().getModel(eModelName::WorkerMineral).render(
			//shaderHandler, { m_position.Get().x - 0.5f, m_position.Get().y, m_position.Get().z - 0.5f });
	}

	Entity::render(shaderHandler, owningFactionController);
}

void Worker::render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	Entity::render_status_bars(shaderHandler, camera, windowSize);

	if (m_taskTimer.isActive())
	{
		float currentTime = m_taskTimer.getElaspedTime() / m_taskTimer.getExpiredTime();
		m_statbarSprite.render(m_position.Get(), windowSize, WORKER_PROGRESS_BAR_WIDTH,
			WORKER_PROGRESS_BAR_WIDTH * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT,
			WORKER_PROGRESS_BAR_YOFFSET, shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

void Worker::renderBuildingCommands(ShaderHandler& shaderHandler) const
{
	for (const auto& building : m_buildQueue)
	{
		building.model.get().render(
			shaderHandler, m_owningFaction->getController(), building.position, glm::vec3(0.0f), false);
	}
}

#ifdef RENDER_PATHING
void Worker::render_path(ShaderHandler& shaderHandler)
{
	RenderPrimitiveMesh::render(shaderHandler, m_movement.path, m_movement.pathMesh);
}
#endif //RENDER_PATHING

void Worker::switchTo(eWorkerState newState)
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
			Level::add_event(GameEvent::create<EntityIdleEvent>({ getID(), m_owningFaction->getController() }));
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
		assert(m_mineralToHarvest);
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
		assert(m_repairTargetEntity);
		m_taskTimer.resetExpirationTime(REPAIR_EXPIRATION_TIME);
		m_taskTimer.setActive(true);
		m_movement.path.clear();
		break;
	default:
		assert(false);
	}

	m_taskTimer.resetElaspedTime();
	
}

bool Worker::move_to(const glm::vec3& destination, const Map& map, eWorkerState state)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position.Get());
	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map));
	if (!m_movement.path.empty())
	{
		switchTo(state);
		return true;
	}
	else
	{
		if (previousDestination != m_position.Get())
		{
			m_movement.path.push_back(previousDestination);
			switchTo(state);
			return true;
		}
		else
		{
			switchTo(eWorkerState::Idle);
		}
	}

	return false;
}

bool Worker::move_to(const glm::vec3& destination, const Map& map, const AABB& ignoreAABB, eWorkerState state)
{
	glm::vec3 previousDestination = Globals::getNextPathDestination(m_movement.path, m_position.Get());
	PathFinding::getInstance().getPathToPosition(*this, destination, m_movement.path, map, createAdjacentPositions(map, ignoreAABB));
	if (!m_movement.path.empty())
	{
		switchTo(state);
		return true;
	}
	else
	{
		if (previousDestination != m_position.Get())
		{
			m_movement.path.push_back(previousDestination);
			switchTo(state);
			return true;
		}
		else
		{
			switchTo(eWorkerState::Idle);
		}
	}

	return false;
}