#include "Worker.h"
#include "Map.h"
#include "UnitSpawnerBuilding.h"
#include "Mineral.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "Faction.h"
#include "SupplyDepot.h"
#include "GameEventHandler.h"

namespace
{
	constexpr float HARVEST_TIME = 2.0f;
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr int RESOURCE_CAPACITY = 30;
	constexpr int RESOURCE_INCREMENT = 10;
}

Worker::Worker(const Faction& owningFaction, const glm::vec3& startingPosition)
	: Unit(owningFaction, startingPosition, eEntityType::Worker),
	m_buildingCommand(),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME, false),
	m_mineralToHarvest(nullptr)
{}

Worker::Worker(const Faction& owningFaction, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Map & map)
	: Unit(owningFaction, startingPosition, eEntityType::Worker),
	m_buildingCommand(),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME, false),
	m_mineralToHarvest(nullptr)
{
	moveTo(destinationPosition, map,
		[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
}

bool Worker::isHoldingResources() const
{
	return m_currentResourceAmount > 0;
}

int Worker::extractResources(const Map& map)
{
	assert(isHoldingResources());
	if (m_mineralToHarvest)
	{
		glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
			m_mineralToHarvest->getAABB(), m_mineralToHarvest->getPosition(), map);

		moveTo(destination, map,
			[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
			eUnitState::MovingToMinerals, m_mineralToHarvest);
	}
	else
	{
		m_currentState = eUnitState::Idle;
	}

	int resources = m_currentResourceAmount;
	m_currentResourceAmount = 0;
	return resources;
}

bool Worker::build(const std::function<const Entity*(Worker&)>& buildingCommand, const glm::vec3& buildPosition, const Map& map)
{
	if (!m_buildingCommand)
	{
		m_buildingCommand = buildingCommand;
		moveTo(Globals::convertToMiddleGridPosition(buildPosition), map, 
			[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, eUnitState::MovingToBuildingPosition);

		return true;
	}

	return false;
}

void Worker::update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, const Faction& opposingFaction,
	const std::list<Unit>& units)
{
	Unit::update(deltaTime, opposingFaction, map, units);

	switch (m_currentState)
	{
	case eUnitState::MovingToMinerals:
		if (m_pathToPosition.empty())
		{
			m_currentState = eUnitState::Harvesting;
		}
		break;
	case eUnitState::ReturningMineralsToHQ:
		assert(isHoldingResources());
		if (m_pathToPosition.empty())
		{
			GameEventHandler::getInstance().addEvent({ eGameEventType::AddResources, m_owningFaction.getName(), getID() });
			m_currentState = eUnitState::Idle;
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

			glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
				HQ.getAABB(), HQ.getPosition(), map);
			moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, eUnitState::ReturningMineralsToHQ);
		}
		break;
	case eUnitState::MovingToBuildingPosition:
		assert(m_buildingCommand);
		if (m_pathToPosition.empty())
		{
			m_currentState = eUnitState::Building;
		}
		break;
	case eUnitState::Building:
		assert(m_pathToPosition.empty() && m_buildingCommand);
		const Entity* newBuilding = m_buildingCommand(*this);
		m_buildingCommand = nullptr;
		if (newBuilding)
		{
			GameEventHandler::getInstance().addEvent({ eGameEventType::RemovePlannedBuilding, m_owningFaction.getName(), getID() });

			glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
				newBuilding->getAABB(), newBuilding->getPosition(), map);
			moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
			assert(!m_pathToPosition.empty());
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
		break;
	}
}

void Worker::moveTo(const glm::vec3& destinationPosition, const Map& map, const GetAllAdjacentPositions& getAdjacentPositions,
	eUnitState state, const Mineral* mineralToHarvest)
{
	assert(state == eUnitState::Moving || state == eUnitState::MovingToBuildingPosition || 
		state == eUnitState::MovingToMinerals || state == eUnitState::ReturningMineralsToHQ);

	if (m_buildingCommand && (state == eUnitState::Moving || state == eUnitState::MovingToMinerals))
	{
		GameEventHandler::getInstance().addEvent({ eGameEventType::RemovePlannedBuilding, m_owningFaction.getName(), getID() });
		m_buildingCommand = nullptr;
	}
	else if (state == eUnitState::Moving)
	{
		m_mineralToHarvest = nullptr;
	}

	if(mineralToHarvest)
	{
		m_mineralToHarvest = mineralToHarvest;
	}

	glm::vec3 previousClosestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		previousClosestDestination = m_pathToPosition.back();
	}

	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition,
		[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, true);
	if (!m_pathToPosition.empty())
	{
		m_currentState = state;
	}
	else
	{
		if (previousClosestDestination != m_position)
		{
			m_pathToPosition.push_back(previousClosestDestination);
			m_currentState = state;
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
	}
}

void Worker::render(ShaderHandler& shaderHandler) const
{
	if (m_currentResourceAmount > 0 && m_currentState != eUnitState::Harvesting)
	{
		ModelManager::getInstance().getModel(eModelName::WorkerMineral).render(
			shaderHandler, { m_position.x - 0.5f, m_position.y, m_position.z - 0.5f });
	}

	Entity::render(shaderHandler);
}