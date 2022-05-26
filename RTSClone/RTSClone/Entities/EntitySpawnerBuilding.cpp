#include "EntitySpawnerBuilding.h"
#include "Core/Camera.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Graphics/Model.h"
#include "Core/Globals.h"
#include "Graphics/ModelManager.h"
#include "Factions/Faction.h"
#include "Core/Map.h"
#include "Graphics/ShaderHandler.h"
#include "Core/Level.h"

EntitySpawnerBuilding::EntitySpawnerBuilding(const glm::vec3& position, const eEntityType type, 
	const int health, const int shield, EntitySpawnerDetails spawnDetails,
	std::function<Entity* (Faction& owningFaction, const Map&, const EntitySpawnerBuilding&)> spawnCallback)
	: Entity(ModelManager::getInstance().getModel(type), position, type, health, shield),
	m_details(spawnDetails),
	m_timer(spawnDetails.timeBetweenSpawn, false),
	m_spawnCallback(spawnCallback)
{
	broadcast<GameMessages::AddAABBToMap>({ m_AABB });
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

EntitySpawnerBuilding::~EntitySpawnerBuilding()
{
	if (getID() != INVALID_ENTITY_ID)
	{
		broadcast<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}	
}

int EntitySpawnerBuilding::get_current_spawn_count() const
{
	return m_spawnCount;
}

std::optional<glm::vec3> EntitySpawnerBuilding::get_waypoint() const
{
	return m_waypoint;
}

bool EntitySpawnerBuilding::is_singular_selectable_only() const
{
	return true;
}

void EntitySpawnerBuilding::update(const float deltaTime, Faction& owningFaction, const Map& map)
{
	Entity::update(deltaTime);
	m_timer.update(deltaTime);
	if (m_timer.isExpired() && m_spawnCount > 0)
	{
		m_timer.resetElaspedTime();
		--m_spawnCount;

		const Entity* spawnedEntity = m_spawnCallback(owningFaction, map, *this);
		if (!spawnedEntity)
		{
			m_spawnCount = 0;
			m_timer.setActive(false);
		}
		else
		{
			if (m_spawnCount == 0)
			{
				m_timer.setActive(false);
			}
			else if (!owningFaction.isAffordable(m_spawnCount * m_details.resourceCost) ||
				owningFaction.isExceedPopulationLimit(m_spawnCount * m_details.populationCost))
			{
				m_spawnCount = { 0 };
				m_timer.setActive(false);
			}
		}
	}
}

void EntitySpawnerBuilding::render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	Entity::render_status_bars(shaderHandler, camera, windowSize);
	if (m_timer.isActive())
	{
		const float currentTime = m_timer.getElaspedTime() / m_timer.getExpiredTime();
		const float width = m_details.progressBarWidth;
		const float yOffset = m_details.progressBarYOffset;

		m_statbarSprite.render(m_position, windowSize, width, width * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

bool EntitySpawnerBuilding::set_waypoint_position(const glm::vec3& position, const Map& map)
{
	if (map.isWithinBounds(position))
	{
		if (m_AABB.contains(position))
		{
			m_waypoint = std::nullopt;
		}
		else
		{
			m_waypoint = position;
			return true;
		}
	}

	return false;
}

bool EntitySpawnerBuilding::add_entity_to_spawn_queue(const Faction& owningFaction)
{
	if (m_spawnCount < m_details.maxSpawnCount &&
		owningFaction.isAffordable(m_spawnCount * m_details.resourceCost + m_details.resourceCost) &&
		!owningFaction.isExceedPopulationLimit(m_spawnCount * m_details.populationCost + m_details.populationCost))
	{
		++m_spawnCount;
		m_timer.setActive(true);
		return true;
	}

	return false;
}

void EntitySpawnerBuilding::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	Entity::render(shaderHandler, owningFactionController);
	if (isSelected() && m_waypoint)
	{
		ModelManager::getInstance().getModel(WAYPOINT_MODEL_NAME).render(shaderHandler, *m_waypoint);
	}
}