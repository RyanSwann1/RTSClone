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
	glm::vec3 getSpawnPosition(const AABB& AABB, const glm::vec3& direction, const glm::vec3& startingPosition)
	{
		glm::vec3 position = startingPosition;
		float distance = 1.0f;
		while (AABB.contains(position))
		{
			position = position + direction * distance;
			++distance;
		}
		
		return position;
	}
}

EntitySpawnerBuilding::~EntitySpawnerBuilding()
{
	broadcastToMessenger<GameMessages::RemoveFromMap>({ m_AABB });
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

glm::vec3 EntitySpawnerBuilding::getUnitSpawnPosition() const
{
	if (isWaypointActive())
	{
		return getSpawnPosition(m_AABB, glm::normalize(m_waypointPosition - m_position), m_position);
	}
	else
	{
		return getSpawnPosition(m_AABB,
			glm::normalize(glm::vec3(Globals::getRandomNumber(-1.0f, 1.0f), Globals::GROUND_HEIGHT, Globals::getRandomNumber(-1.0f, 1.0f))),
			m_position);
	}
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
		m_owningFaction.isAffordable(static_cast<int>(m_spawnQueue.size()) * resourceCost + resourceCost) &&
		!m_owningFaction.isExceedPopulationLimit(static_cast<int>(m_spawnQueue.size()) * populationCost + populationCost);
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
			else if (!m_owningFaction.isAffordable(static_cast<int>(m_spawnQueue.size()) * resourceCost) ||
				m_owningFaction.isExceedPopulationLimit(static_cast<int>(m_spawnQueue.size()) * populationCost))
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