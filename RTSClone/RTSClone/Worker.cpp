#include "Worker.h"
#include "Map.h"
#include "UnitSpawnerBuilding.h"
#include "Mineral.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "Faction.h"
#include "SupplyDepot.h"
#include "GameEventHandler.h"
#include "GameEvent.h"
#include "FactionHandler.h"

namespace
{
	constexpr float HARVEST_TIME = 2.0f;
	constexpr float REPAIR_TIME = 3.5f;
	constexpr float BUILD_TIME = 2.0f;
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr int RESOURCE_CAPACITY = 30;
	constexpr int RESOURCE_INCREMENT = 10;
	constexpr float MINIMUM_REPAIR_DISTANCE = static_cast<float>(Globals::NODE_SIZE) * 4.0f;
}

//BuildingCommand
BuildingCommand::BuildingCommand(const std::function<const Entity*()>& command, const glm::vec3& buildPosition)
	: command(command),
	buildPosition(buildPosition)
{
	assert(command);
}

//Worker
Worker::Worker(const Faction& owningFaction, const glm::vec3& startingPosition)
	: Unit(owningFaction, startingPosition, eEntityType::Worker, Globals::WORKER_STARTING_HEALTH, 
		ModelManager::getInstance().getModel(WORKER_MODEL_NAME)),
	m_buildingCommands(),
	m_repairTargetEntityID(Globals::INVALID_ENTITY_ID),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME, false),
	m_buildTimer(BUILD_TIME, false),
	m_repairTimer(REPAIR_TIME, false),
	m_mineralToHarvest(nullptr)
{}

Worker::Worker(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Map & map)
	: Unit(owningFaction, startingPosition, eEntityType::Worker, Globals::WORKER_STARTING_HEALTH,
		ModelManager::getInstance().getModel(WORKER_MODEL_NAME)),
	m_buildingCommands(),
	m_repairTargetEntityID(Globals::INVALID_ENTITY_ID),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME, false),
	m_buildTimer(BUILD_TIME, false),
	m_repairTimer(REPAIR_TIME, false),
	m_mineralToHarvest(nullptr)
{
	moveTo(destinationPosition, map,
		[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

Worker::~Worker()
{
	GameEventHandler::getInstance().gameEvents.push(GameEvent::createRemoveAllWorkerPlannedBuildings(m_owningFaction.getController(), getID()));
}

const Timer& Worker::getBuildTimer() const
{
	return m_buildTimer;
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

void Worker::setBuildingToRepair(const Entity& building, const Map& map)
{
	assert(Globals::BUILDING_TYPES.isMatch(building.getEntityType()));
	
	m_repairTargetEntityID = building.getID();

	if (Globals::getSqrDistance(building.getPosition(), m_position) > MINIMUM_REPAIR_DISTANCE * MINIMUM_REPAIR_DISTANCE)
	{
		moveTo(PathFinding::getInstance().getClosestPositionToAABB(building.getPosition(),
			building.getAABB(), map),
			map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
			eUnitState::MovingToRepairPosition);
	}
}

bool Worker::build(const std::function<const Entity*()>& buildingCommand, const glm::vec3& buildPosition, const Map& map)
{
	if (!map.isPositionOccupied(buildPosition))
	{
		m_buildingCommands.emplace(buildingCommand, Globals::convertToMiddleGridPosition(buildPosition));
		if (m_buildingCommands.size() == size_t(1))
		{
			moveTo(Globals::convertToMiddleGridPosition(buildPosition), map,
				[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eUnitState::MovingToBuildingPosition);
		}

		return true;
	}

	return false;
}

void Worker::update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, FactionHandler& factionHandler,
	const Timer& unitStateHandlerTimer)
{
	Unit::update(deltaTime, factionHandler, map, unitStateHandlerTimer);

	switch (getCurrentState())
	{
	case eUnitState::MovingToMinerals:
		if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Harvesting, map);
		}
		break;
	case eUnitState::ReturningMineralsToHQ:
		assert(isHoldingResources());
		if (m_pathToPosition.empty())
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::createAddResources(m_owningFaction.getController(), getID()));
			if (m_mineralToHarvest)
			{
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position,
					m_mineralToHarvest->getAABB(), map);

				moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
					eUnitState::MovingToMinerals, m_mineralToHarvest);
			}
			else
			{
				switchToState(eUnitState::Idle, map);
			}
		}
		break;
	case eUnitState::Harvesting:
		assert(m_currentResourceAmount <= RESOURCE_CAPACITY);
		if (m_currentResourceAmount < RESOURCE_CAPACITY)
		{
			m_harvestTimer.setActive(true);
			m_harvestTimer.update(deltaTime);

			if (m_harvestTimer.isExpired())
			{
				m_harvestTimer.resetElaspedTime();
				m_currentResourceAmount += RESOURCE_INCREMENT;
			}
		}

		if (m_currentResourceAmount == RESOURCE_CAPACITY)
		{
			m_harvestTimer.setActive(false);
			m_pathToPosition.clear();

			glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position,
				HQ.getAABB(), map);
			moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, 
				eUnitState::ReturningMineralsToHQ);
		}
		break;
	case eUnitState::MovingToBuildingPosition:
		assert(!m_buildingCommands.empty());
		if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Building, map);
		}
		break;
	case eUnitState::Building:
	{
		assert(m_pathToPosition.empty() && !m_buildingCommands.empty());
		m_buildTimer.update(deltaTime);
		m_buildTimer.setActive(true);
		if (!m_buildTimer.isExpired())
		{
			break;
		}

		m_buildTimer.resetElaspedTime();
		m_buildTimer.setActive(false);
		const Entity* newBuilding = m_buildingCommands.front().command();
		m_buildingCommands.pop();
		if (newBuilding)
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::createRemovePlannedBuilding(m_owningFaction.getController(), newBuilding->getPosition()));
		}
		else
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::createRemoveAllWorkerPlannedBuildings(m_owningFaction.getController(), getID()));
			clearBuildingCommands();
		}

		if (!m_buildingCommands.empty())
		{
			moveTo(m_buildingCommands.front().buildPosition, map,
				[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eUnitState::MovingToBuildingPosition);
		}
		else
		{
			if (newBuilding)
			{
				moveTo(PathFinding::getInstance().getRandomAvailablePositionOutsideAABB(*this, map), 
					map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
			}
			else
			{
				switchToState(eUnitState::Idle, map);
			}
		}
	}

		break;
	case eUnitState::MovingToRepairPosition:
		if (m_pathToPosition.empty())
		{
			switchToState(eUnitState::Repairing, map);
		}
		break;
	case eUnitState::Repairing:
		assert(m_pathToPosition.empty() && m_repairTargetEntityID != Globals::INVALID_ENTITY_ID);
		m_repairTimer.setActive(true);
		m_repairTimer.update(deltaTime);
		if (m_repairTimer.isExpired())
		{	
			m_repairTimer.resetElaspedTime();

			assert(factionHandler.isFactionActive(m_owningFaction.getController()));
			const Entity* targetEntity = factionHandler.getFaction(m_owningFaction.getController()).getEntity(m_repairTargetEntityID);
			if (targetEntity && targetEntity->getHealth() < targetEntity->getMaximumHealth())
			{
				m_rotation.y = Globals::getAngle(targetEntity->getPosition(), m_position);
				
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createRepairEntity(
					m_owningFaction.getController(), m_repairTargetEntityID));
			}
			else
			{
				m_repairTimer.setActive(false);
				switchToState(eUnitState::Idle, map);
				m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
			}
		}
		break;
	}
}

void Worker::moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
	eUnitState state, const Mineral* mineralToHarvest)
{
	assert(state == eUnitState::Moving || state == eUnitState::MovingToBuildingPosition || 
		state == eUnitState::MovingToMinerals || state == eUnitState::ReturningMineralsToHQ || 
		state == eUnitState::MovingToRepairPosition);

	if (state != eUnitState::MovingToBuildingPosition && !m_buildingCommands.empty())
	{
		GameEventHandler::getInstance().gameEvents.push(GameEvent::createRemoveAllWorkerPlannedBuildings(
			m_owningFaction.getController(), getID()));
		 
		m_buildTimer.resetElaspedTime();
		clearBuildingCommands();
	}
	else if (state == eUnitState::Moving)
	{
		m_mineralToHarvest = nullptr;
	}

	if(mineralToHarvest)
	{
		m_mineralToHarvest = mineralToHarvest;
	}

	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, adjacentPositions,
		map);
	if (!m_pathToPosition.empty())
	{
		switchToState(state, map);
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			switchToState(state, map);
		}
		else
		{
			switchToState(eUnitState::Idle, map);
		}
	}
}

void Worker::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	if (m_currentResourceAmount > 0 && getCurrentState() != eUnitState::Harvesting)
	{
		//ModelManager::getInstance().getModel(eModelName::WorkerMineral).render(
			//shaderHandler, { m_position.x - 0.5f, m_position.y, m_position.z - 0.5f });
	}

	Entity::render(shaderHandler, owningFactionController);
}

void Worker::clearBuildingCommands()
{
	std::queue<BuildingCommand> empty;
	std::swap(m_buildingCommands, empty);
}