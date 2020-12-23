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
#include "ShaderHandler.h"
#include "Camera.h"
#ifdef RENDER_PATHING
#include "RenderPathMesh.h"
#endif // RENDER_PATHING

namespace
{
	const float HARVEST_TIME = 2.0f;
	const float REPAIR_TIME = 3.5f;
	const float BUILD_TIME = 2.0f;
	const float MOVEMENT_SPEED = 7.5f;
	const int RESOURCE_CAPACITY = 30;
	const int RESOURCE_INCREMENT = 10;
	const float REPAIR_DISTANCE = static_cast<float>(Globals::NODE_SIZE);
	const float WORKER_PROGRESS_BAR_WIDTH = 60.0f;
	const float WORKER_PROGRESS_BAR_YOFFSET = 30.0f;
}

//BuildingCommand
BuildingCommand::BuildingCommand(const std::function<const Entity*()>& command, const glm::vec3& buildPosition, eEntityType entityType)
	: command(command),
	buildPosition(buildPosition),
	m_model(ModelManager::getInstance().getModel(entityType))
{
	assert(command);
}

//Worker
Worker::Worker(const Faction& owningFaction, const glm::vec3& startingPosition)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, 
		Globals::WORKER_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
	m_buildingCommands(),
	m_repairTargetEntityID(Globals::INVALID_ENTITY_ID),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME, false),
	m_buildTimer(BUILD_TIME, false),
	m_repairTimer(REPAIR_TIME, false),
	m_mineralToHarvest(nullptr)
{}

Worker::Worker(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Map & map)
	: Entity(ModelManager::getInstance().getModel(WORKER_MODEL_NAME), startingPosition, eEntityType::Worker, Globals::WORKER_STARTING_HEALTH,
		owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_currentState(eWorkerState::Idle),
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

const std::vector<glm::vec3>& Worker::getPathToPosition() const
{
	return m_pathToPosition;
}

eWorkerState Worker::getCurrentState() const
{
	return m_currentState;
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

void Worker::setEntityToRepair(const Entity& entity, const Map& map)
{
	m_repairTargetEntityID = entity.getID();
	m_repairTimer.resetElaspedTime();

	moveTo(PathFinding::getInstance().getClosestPositionToAABB(m_position,
		entity.getAABB(), map),
		map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
		eWorkerState::MovingToRepairPosition);

	if (m_pathToPosition.empty())
	{
		m_currentState = eWorkerState::Repairing;
	}
}

bool Worker::build(const std::function<const Entity*()>& buildingCommand, const glm::vec3& buildPosition, 
	const Map& map, eEntityType entityType)
{
	if (!map.isPositionOccupied(buildPosition))
	{
		m_buildingCommands.emplace_back(buildingCommand, Globals::convertToMiddleGridPosition(buildPosition), entityType);
		if (m_buildingCommands.size() == 1)
		{
			moveTo(Globals::convertToMiddleGridPosition(buildPosition), map,
				[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eWorkerState::MovingToBuildingPosition);
		}

		return true;
	}

	return false;
}

void Worker::update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, FactionHandler& factionHandler,
	const Timer& unitStateHandlerTimer)
{
	Entity::updateShieldReplenishTimer(deltaTime);

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
			m_currentState = eWorkerState::Idle;
		}
		break;
	case eWorkerState::MovingToMinerals:
		if (m_pathToPosition.empty())
		{
			m_currentState = eWorkerState::Harvesting;
		}
		break;
	case eWorkerState::ReturningMineralsToHQ:
		assert(isHoldingResources());
		if (m_pathToPosition.empty())
		{
			GameEventHandler::getInstance().gameEvents.push(GameEvent::createAddResources(m_owningFaction.getController(), getID()));
			if (m_mineralToHarvest)
			{
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position,
					m_mineralToHarvest->getAABB(), map);
				moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
					eWorkerState::MovingToMinerals, m_mineralToHarvest);
			}
			else
			{
				m_currentState = eWorkerState::Idle;
			}
		}
		break;
	case eWorkerState::Harvesting:
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
				eWorkerState::ReturningMineralsToHQ);
		}
		break;
	case eWorkerState::MovingToBuildingPosition:
		assert(!m_buildingCommands.empty());
		if (m_pathToPosition.empty())
		{
			m_currentState = eWorkerState::Building;
		}
		break;
	case eWorkerState::Building:
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
		m_buildingCommands.pop_front();
		if (!newBuilding)
		{
			m_buildingCommands.clear();
		}

		if (!m_buildingCommands.empty())
		{
			moveTo(m_buildingCommands.front().buildPosition, map,
				[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, eWorkerState::MovingToBuildingPosition);
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
				m_currentState = eWorkerState::Idle;
			}
		}
	}
	break;
	case eWorkerState::MovingToRepairPosition:
		if (m_pathToPosition.empty())
		{
			m_currentState = eWorkerState::Repairing;
		}
		break;
	case eWorkerState::Repairing:
		assert(m_pathToPosition.empty() && m_repairTargetEntityID != Globals::INVALID_ENTITY_ID);
		m_repairTimer.setActive(true);
		m_repairTimer.update(deltaTime);
		if (m_repairTimer.isExpired())
		{
			m_repairTimer.resetElaspedTime();
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
					m_repairTimer.setActive(false);
					m_currentState = eWorkerState::Idle;
					m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
				}
			}
			else
			{
				m_repairTimer.setActive(false);
				m_currentState = eWorkerState::Idle;
				m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
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
				m_repairTimer.setActive(false);
				m_currentState = eWorkerState::Idle;
				m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
			}
		}
		break;
	}
}

void Worker::moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
	eWorkerState state, const Mineral* mineralToHarvest)
{
	if (state != eWorkerState::MovingToBuildingPosition && !m_buildingCommands.empty())
	{		 
		m_buildTimer.resetElaspedTime();
		m_buildingCommands.clear();
	}
	else if (state == eWorkerState::Moving)
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
		map, m_owningFaction);
	if (!m_pathToPosition.empty())
	{
		m_currentState = state;
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			m_currentState = state;
		}
		else
		{
			m_currentState = eWorkerState::Idle;
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
	if (m_buildTimer.isActive() || m_harvestTimer.isActive() || m_repairTimer.isActive())
	{
		float currentTime = 0.0f;
		switch (m_currentState)
		{
		case eWorkerState::Building:
			currentTime = m_buildTimer.getElaspedTime() / m_buildTimer.getExpiredTime();
			break;
		case eWorkerState::Harvesting:
			currentTime = m_harvestTimer.getElaspedTime() / m_harvestTimer.getExpiredTime();
			break;
		case eWorkerState::Repairing:
			currentTime = m_repairTimer.getElaspedTime() / m_repairTimer.getExpiredTime();
			break;
		}

		m_statbarSprite.render(m_position, windowSize, WORKER_PROGRESS_BAR_WIDTH, 
			WORKER_PROGRESS_BAR_WIDTH * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT,
			WORKER_PROGRESS_BAR_YOFFSET, shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

void Worker::renderBuildingCommands(ShaderHandler& shaderHandler) const
{
	for (const auto& buildingCommand : m_buildingCommands)
	{
		buildingCommand.m_model.get().render(shaderHandler, buildingCommand.buildPosition);
	}
}

void Worker::renderPathMesh(ShaderHandler& shaderHandler)
{
	RenderPathMesh::render(shaderHandler, m_pathToPosition, m_renderPathMesh);
}