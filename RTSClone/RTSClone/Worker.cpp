#include "Worker.h"
#include "Map.h"
#include "BuildingSpawner.h"
#include "Mineral.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "Faction.h"
#include "SupplyDepot.h"

namespace
{
	constexpr float HARVEST_TIME = 2.0f;
	constexpr float MOVEMENT_SPEED = 7.5f;
	constexpr int RESOURCE_CAPACITY = 30;
	constexpr int RESOURCE_INCREMENT = 10;
}

Worker::Worker(int ID, const glm::vec3& startingPosition, Map& map)
	: Unit(ID, startingPosition, map, eModelName::Worker, eEntityType::Worker),
	m_buildingCommand(),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
{}

Worker::Worker(int ID, const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, Map & map)
	: Unit(ID, startingPosition, map, eModelName::Worker, eEntityType::Worker),
	m_buildingCommand(),
	m_currentResourceAmount(0),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
{
	moveTo(destinationPosition, map);
}

bool Worker::isHoldingResources() const
{
	return m_currentResourceAmount > 0;
}

int Worker::extractResources()
{
	assert(m_currentResourceAmount > 0);
	int resources = m_currentResourceAmount;
	m_currentResourceAmount = 0;
	return resources;
}

void Worker::build(const std::function<const Entity*(Worker&)>& buildingCommand, const glm::vec3& destination, const Map& map)
{
	m_buildingCommand = buildingCommand;
	moveTo(Globals::convertToMiddleGridPosition(destination), map, eUnitState::MovingToBuildingPosition);
}

void Worker::update(float deltaTime, const BuildingSpawner& HQ, const Map& map, Faction& owningFaction)
{
	Unit::update(deltaTime);

	switch (m_currentState)
	{
	case eUnitState::MovingToMinerals:
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.update(m_position);
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();

				if (m_pathToPosition.empty())
				{
					m_currentState = eUnitState::Harvesting;
				}
			}
		}
		else
		{
			assert(false);
			m_currentState = eUnitState::Idle;
		}
		break;
	case eUnitState::ReturningMineralsToHQ:
		assert(isHoldingResources());
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.update(m_position);
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();
				if (m_pathToPosition.empty())
				{
					owningFaction.addResources(*this);
					if (m_mineralToHarvest)
					{
						glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
							m_mineralToHarvest->getAABB(), m_mineralToHarvest->getPosition(), map);
						PathFinding::getInstance().getPathToPosition(*this, destination, m_pathToPosition,
							[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, true);
					}

					if (!m_pathToPosition.empty())
					{
						m_currentState = eUnitState::MovingToMinerals;
					}
					else
					{
						m_currentState = eUnitState::Idle;
					}
				}
			}
		}
		else
		{
			assert(false);
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
			PathFinding::getInstance().getPathToPosition(*this, destination, m_pathToPosition,
				[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, true);
			
			if (!m_pathToPosition.empty())
			{
				m_currentState = eUnitState::ReturningMineralsToHQ;
			}
			else
			{
				m_currentState = eUnitState::Idle;
			}
		}
		
		break;
	case eUnitState::MovingToBuildingPosition:
		assert(m_buildingCommand);
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.update(m_position);
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();

				if (m_pathToPosition.empty())
				{
					m_currentState = eUnitState::Building;
				}
			}
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
		break;
	case eUnitState::Building:
		assert(m_pathToPosition.empty() && m_buildingCommand);
		const Entity* newBuilding = m_buildingCommand(*this);
		m_buildingCommand = nullptr;
		if (newBuilding)
		{
			glm::vec3 position = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
				newBuilding->getAABB(), newBuilding->getPosition(), map);
			moveTo(position, map);
			
			assert(!m_pathToPosition.empty());
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
		
		break;
	}
}

void Worker::moveTo(const glm::vec3& destinationPosition, const Map& map, eUnitState state)
{
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

void Worker::moveTo(const glm::vec3 & destinationPosition, const Map & map, const std::vector<Mineral>& minerals)
{
	for (const auto& mineral : minerals)
	{
		if (mineral.getAABB().contains(destinationPosition))
		{
			m_mineralToHarvest = &mineral;
			m_pathToPosition.clear();
			
			glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(m_position,
				m_mineralToHarvest->getAABB(), m_mineralToHarvest->getPosition(), map);
			PathFinding::getInstance().getPathToPosition(*this, destination, m_pathToPosition,
				[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, true);

			if (!m_pathToPosition.empty())
			{
				m_currentState = eUnitState::MovingToMinerals;
			}
			else
			{
				m_currentState = eUnitState::Idle;
			}

			break;
		}
		else
		{
			m_mineralToHarvest = nullptr;
		}
	}

	if (!m_mineralToHarvest)
	{
		moveTo(destinationPosition, map);
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