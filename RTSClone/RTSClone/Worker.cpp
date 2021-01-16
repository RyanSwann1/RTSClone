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
	m_taskTimer(0.0f, false),
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
	m_taskTimer(0.0f, false),
	m_mineralToHarvest(nullptr)
{
	moveTo(destinationPosition, map,
		[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
}

const std::deque<BuildingCommand>& Worker::getBuildingCommands() const
{
	return m_buildingCommands;
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
	m_repairTargetEntityID = entity.getID();

	glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, entity.getAABB(), map);
	moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
		eWorkerState::MovingToRepairPosition);

	if (m_pathToPosition.empty())
	{
		switchTo(eWorkerState::Repairing);
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
				switchTo(eWorkerState::Idle);
			}
		}
		break;
	case eWorkerState::Harvesting:
		assert(m_currentResourceAmount <= RESOURCE_CAPACITY && m_taskTimer.isActive());
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
			glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(m_position, HQ.getAABB(), map);
			moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
				eWorkerState::ReturningMineralsToHQ);
		}
		break;
	case eWorkerState::MovingToBuildingPosition:
		assert(!m_buildingCommands.empty());
		if (m_pathToPosition.empty())
		{
			switchTo(eWorkerState::Building);
		}
		break;
	case eWorkerState::Building:
	{
		assert(m_pathToPosition.empty() && !m_buildingCommands.empty() && m_taskTimer.isActive());
		if (!m_taskTimer.isExpired())
		{
			break;
		}

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
				switchTo(eWorkerState::Idle);
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
					m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
				}
			}
			else
			{
				switchTo(eWorkerState::Idle);
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
				switchTo(eWorkerState::Idle);
				m_repairTargetEntityID = Globals::INVALID_ENTITY_ID;
			}
		}
		break;
	}

	if (m_taskTimer.isActive())
	{
		m_taskTimer.update(deltaTime);
	}
}

void Worker::moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
	eWorkerState state, const Mineral* mineralToHarvest)
{
	if(mineralToHarvest)
	{
		assert(state == eWorkerState::MovingToMinerals);
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
		switchTo(state);
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
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
	for (const auto& buildingCommand : m_buildingCommands)
	{
		buildingCommand.m_model.get().render(shaderHandler, buildingCommand.buildPosition, m_owningFaction.getController());
	}
}

void Worker::renderPathMesh(ShaderHandler& shaderHandler)
{
	RenderPathMesh::render(shaderHandler, m_pathToPosition, m_renderPathMesh);
}

void Worker::switchTo(eWorkerState newState)
{
	//On Exit Current State
	switch (m_currentState)
	{
	case eWorkerState::Idle:
	case eWorkerState::Moving:
		break;
	case eWorkerState::MovingToMinerals:
	case eWorkerState::ReturningMineralsToHQ:
	case eWorkerState::Harvesting:
		if (newState != eWorkerState::MovingToMinerals && 
			newState != eWorkerState::ReturningMineralsToHQ &&
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
			m_buildingCommands.clear();
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
	case eWorkerState::MovingToMinerals:
	case eWorkerState::ReturningMineralsToHQ:
	case eWorkerState::MovingToBuildingPosition:
	case eWorkerState::MovingToRepairPosition:
		m_taskTimer.setActive(false);
		break;
	case eWorkerState::Harvesting:
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