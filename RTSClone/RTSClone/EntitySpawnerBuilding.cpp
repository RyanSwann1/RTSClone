#include "EntitySpawnerBuilding.h"
#include "Camera.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Model.h"
#include "Globals.h"
#include "ModelManager.h"
#include "Faction.h"
#include "Map.h"
#include "ShaderHandler.h"

namespace
{	
	const int MAX_SPAWN_DISTANCE = 20;

	bool getWaypointSpawnPosition(const EntitySpawnerBuilding& building, const Map& map, glm::vec3& spawnPosition)
	{
		assert(building.isWaypointActive());
		glm::ivec2 startingPosition = Globals::convertToGridPosition(building.getPosition());
		glm::ivec2 waypointPosition = Globals::convertToGridPosition(building.getWaypointPosition());
		bool positionFound = false;
		int distance = glm::ceil<int>(glm::distance(glm::vec2(waypointPosition), glm::vec2(startingPosition)));
		
		for (int i = Globals::NODE_SIZE; i < distance; i += Globals::NODE_SIZE)
		{
			glm::vec3 position = Globals::convertToWorldPosition(startingPosition + (waypointPosition - startingPosition) * i);
			if (!building.getAABB().contains(position) && map.isPositionOccupied(position))
			{
				positionFound = false;
				break;
			}
			else if (!building.getAABB().contains(position))
			{
				positionFound = true;
				spawnPosition = position;
			}
		}

		return positionFound;
	}
}

EntitySpawnerBuilding::~EntitySpawnerBuilding()
{
	if (m_status.isActive())
	{
		broadcastToMessenger<GameMessages::RemoveFromMap>({ m_AABB });
	}
}

const Timer& EntitySpawnerBuilding::getSpawnTimer() const
{
	return m_spawnTimer;
}

int EntitySpawnerBuilding::getCurrentSpawnCount() const
{
	return static_cast<int>(m_spawnQueue.size());
}

bool EntitySpawnerBuilding::isWaypointActive() const
{
	return m_waypointPosition != m_position;
}

const glm::vec3& EntitySpawnerBuilding::getWaypointPosition() const
{
	assert(isWaypointActive());
	return m_waypointPosition;
}

bool EntitySpawnerBuilding::getEntitySpawnPosition(const Map& map, glm::vec3& position, const std::vector<Unit>& units, 
	const std::vector<Worker>& workers) const
{
	bool spawnPositionFound = false;
	if (isWaypointActive() && getWaypointSpawnPosition(*this, map, position))
	{
		spawnPositionFound = true;
	}
	if (!spawnPositionFound && PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(m_position, units, workers, map, position))
	{
		spawnPositionFound = true;
	}

	return spawnPositionFound;
}

void EntitySpawnerBuilding::setWaypointPosition(const glm::vec3& position, const Map& map)
{
	if (map.isWithinBounds(position))
	{
		if (m_AABB.contains(position))
		{
			m_waypointPosition = m_position;
		}
		else
		{
			m_waypointPosition = position;
		}
	}
}

bool EntitySpawnerBuilding::isEntityAddableToSpawnQueue(int maxEntitiesInSpawnQueue, int resourceCost, int populationCost) const
{
	return m_spawnQueue.size() < maxEntitiesInSpawnQueue &&
		m_owningFaction.get().isAffordable(static_cast<int>(m_spawnQueue.size()) * resourceCost + resourceCost) &&
		!m_owningFaction.get().isExceedPopulationLimit(static_cast<int>(m_spawnQueue.size()) * populationCost + populationCost);
}

void EntitySpawnerBuilding::addEntityToSpawnQueue(eEntityType entityType)
{
	m_spawnQueue.push_back(entityType);
	m_spawnTimer.setActive(true);
}

void EntitySpawnerBuilding::update(float deltaTime, int resourceCost, int populationCost,
	int maxEntityInSpawnQueue, const Map& map, FactionHandler& factionHandler)
{
	Entity::update(deltaTime);
	m_spawnTimer.update(deltaTime);
	if (m_spawnTimer.isExpired() && !m_spawnQueue.empty())
	{
		m_spawnTimer.resetElaspedTime();
		
		const Entity* spawnedEntity = spawnEntity(map, factionHandler);
		if (!spawnedEntity)
		{
			m_spawnQueue.clear();
			m_spawnTimer.setActive(false);
		}
		else
		{
			m_spawnQueue.pop_back();
			if (m_spawnQueue.empty())
			{
				m_spawnTimer.setActive(false);
			}
			else if (!m_owningFaction.get().isAffordable(static_cast<int>(m_spawnQueue.size()) * resourceCost) ||
				m_owningFaction.get().isExceedPopulationLimit(static_cast<int>(m_spawnQueue.size()) * populationCost))
			{
				m_spawnQueue.clear();
				m_spawnTimer.setActive(false);
			}
		}
	}
}

void EntitySpawnerBuilding::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	if (isSelected() && isWaypointActive())
	{
		ModelManager::getInstance().getModel(WAYPOINT_MODEL_NAME).render(shaderHandler, m_waypointPosition);
	}

	Entity::render(shaderHandler, owningFactionController);
}

EntitySpawnerBuilding::EntitySpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, 
	float spawnTimerExpirationTime, int health, Faction& owningFaction, const Model& model,
	int maxEntityInSpawnQueue)
	: Entity(model, startingPosition, entityType, health, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_spawnQueue(),
	m_spawnTimer(spawnTimerExpirationTime, false),
	m_waypointPosition(m_position)
{
	broadcastToMessenger<GameMessages::AddToMap>({ m_AABB });
	m_spawnQueue.reserve(static_cast<size_t>(maxEntityInSpawnQueue));
}