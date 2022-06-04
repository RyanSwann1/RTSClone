#include "Entity.h"
#include "Graphics/Model.h"
#include "Graphics/ShaderHandler.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Core/IDGenerator.h"
#include "Core/FactionController.h"
#include "Factions/Faction.h"
#include "Events/GameEvents.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Camera.h"

namespace
{
	constexpr float DEFAULT_STAT_BAR_HEIGHT = 10.0f;
		 
	constexpr float WORKER_HEALTH_BAR_YOFFSET = 35.0f;
	constexpr float UNIT_HEALTH_BAR_YOFFSET = 50.0f;
	constexpr float HQ_HEALTH_BAR_YOFFSET = 225.0f;
	constexpr float SUPPLY_DEPOT_HEALTH_BAR_YOFFSET = 85.0f;
	constexpr float BARRACKS_HEALTH_BAR_YOFFSET = 85.0f;
	constexpr float TURRET_HEALTH_BAR_YOFFSET = 60.0f;
	constexpr float LABORATORY_HEALTH_BAR_YOFFSET = 130.0f;
		 
	constexpr float UNIT_SHIELD_BAR_YOFFSET = 60.0f;
	constexpr float WORKER_SHIELD_BAR_YOFFSET = 45.0f;
	constexpr float HQ_SHIELD_BAR_YOFFSET = 235.0f;
	constexpr float SUPPLY_DEPOT_SHIELD_BAR_YOFFSET = 95.0f;
	constexpr float BARRACKS_SHIELD_BAR_YOFFSET = 95.0f;
	constexpr float TURRET_SHIELD_BAR_YOFFSET = 70.0f;
	constexpr float LABORATORY_SHIELD_BAR_YOFFSET = 140.0f;

	const std::array<float, static_cast<size_t>(eEntityType::Max) + 1> ENTITIES_YOFFSET_HEALTH
	{
		UNIT_HEALTH_BAR_YOFFSET,
		WORKER_HEALTH_BAR_YOFFSET,
		HQ_HEALTH_BAR_YOFFSET,
		SUPPLY_DEPOT_HEALTH_BAR_YOFFSET,
		BARRACKS_HEALTH_BAR_YOFFSET,
		TURRET_HEALTH_BAR_YOFFSET,
		LABORATORY_HEALTH_BAR_YOFFSET
	};

	const std::array<float, static_cast<size_t>(eEntityType::Max) + 1> ENTITIES_YOFFSET_SHIELD
	{
		UNIT_SHIELD_BAR_YOFFSET,
		WORKER_SHIELD_BAR_YOFFSET,
		HQ_SHIELD_BAR_YOFFSET,
		SUPPLY_DEPOT_SHIELD_BAR_YOFFSET,
		BARRACKS_SHIELD_BAR_YOFFSET,
		TURRET_SHIELD_BAR_YOFFSET,
		LABORATORY_SHIELD_BAR_YOFFSET
	};

	const float SHIELD_REPLENISH_TIMER_EXPIRATION = 15.0f;
}

//Entity
Entity::Entity(const Model& model, const Position& position, eEntityType entityType, 
	int health, int shield, const glm::vec3& startingRotation)
	: m_position(position),
	m_rotation(startingRotation),
	m_maximumShield(shield),
	m_shield(m_maximumShield),
	m_maximumHealth(health),
	m_health(m_maximumHealth),
	m_type(entityType),
	m_shieldReplenishTimer(SHIELD_REPLENISH_TIMER_EXPIRATION, false),
	m_model(model)
{
	m_AABB.reset(m_position.Get(), m_model);
	if (m_maximumShield == 1)
	{
		m_shieldReplenishTimer.setActive(true);
	}
}

void Entity::update(float deltaTime)
{
	m_shieldReplenishTimer.update(deltaTime);
	if (m_shieldReplenishTimer.isExpired())
	{
		m_shieldReplenishTimer.resetElaspedTime();
		increaseShield();
	}
}

void Entity::increaseShield()
{
	if (m_shield < m_maximumShield)
	{
		++m_shield;
	}
}

int Entity::getID() const
{
	return m_id.Get();
}

const glm::vec3& Entity::getRotation() const
{
	return m_rotation;
}

eEntityType Entity::getEntityType() const
{
	return m_type;
}

int Entity::getMaximumHealth() const
{
	return m_maximumHealth;
}

int Entity::getHealth() const
{
	return m_health;
}

int Entity::getShield() const
{
	return m_shield;
}

void Entity::takeDamage(const TakeDamageEvent& gameEvent, const Map& map)
{
	if (m_shield > 0)
	{
		m_shield -= gameEvent.damage;
		if (m_shield < 0)
		{
			m_health -= glm::abs(m_shield);
			m_shield = 0;
		}
	}
	else
	{
		m_health -= gameEvent.damage;
	}
}

bool Entity::isDead() const
{
	return m_health <= 0;
}

void Entity::repair()
{
	if (m_health < m_maximumHealth)
	{
		++m_health;
	}
}

void Entity::increaseMaximumShield(const Faction& owningFaction)
{
	m_maximumShield = owningFaction.getCurrentShieldAmount();
	if (m_maximumShield == 1)
	{
		assert(m_shield == 0 && !m_shieldReplenishTimer.isActive());
		m_shield = m_maximumShield;
		m_shieldReplenishTimer.setActive(true);
	}
}

bool Entity::repairEntity(const Entity& entity, const Map& map)
{
	assert(entity.getID() != getID());
	return entity.getHealth() < entity.getMaximumHealth();
}

void Entity::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	switch (owningFactionController)
	{
	case eFactionController::Player:
		m_model.get().render(shaderHandler, owningFactionController, m_position.Get(), m_rotation, m_selected);
		break;
	case eFactionController::AI_1:
	case eFactionController::AI_2:
	case eFactionController::AI_3:
		m_model.get().render(shaderHandler, owningFactionController, m_position.Get(), m_rotation, false);
		break;
	default:
		assert(false);
	}
}

void Entity::renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_selected)
	{
		float width = Globals::ENTITIES_STAT_BAR_WIDTH[static_cast<int>(getEntityType())];
		float yOffset = ENTITIES_YOFFSET_HEALTH[static_cast<int>(getEntityType())];
		
		m_statbarSprite.render(m_position.Get(), windowSize, width, width, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::BACKGROUND_BAR_COLOR);
		
		float currentHealth = static_cast<float>(m_health) / static_cast<float>(m_maximumHealth);
		m_statbarSprite.render(m_position.Get(), windowSize, width, width * currentHealth, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::HEALTH_BAR_COLOR);
	}
}

void Entity::renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_selected && m_maximumShield > 0)
	{
		float width = Globals::ENTITIES_STAT_BAR_WIDTH[static_cast<int>(getEntityType())];
		float yOffset = ENTITIES_YOFFSET_SHIELD[static_cast<int>(getEntityType())];

		m_statbarSprite.render(m_position.Get(), windowSize, width, width, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::BACKGROUND_BAR_COLOR);

		float currentShield = static_cast<float>(m_shield) / static_cast<float>(m_maximumShield);
		m_statbarSprite.render(m_position.Get(), windowSize, width, width * currentShield, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::SHIELD_BAR_COLOR);
	}
}

void Entity::render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	renderHealthBar(shaderHandler, camera, windowSize);
	renderShieldBar(shaderHandler, camera, windowSize);
}

void Entity::setPosition(const glm::vec3& position)
{
	m_AABB.update(m_position.Set(position));
}

const glm::vec3& Entity::getPosition() const
{
	return m_position.Get();
}

const AABB& Entity::getAABB() const
{
	return m_AABB;
}

bool Entity::isSelected() const
{
	return m_selected;
}

bool Entity::setSelected(bool selected)
{
	m_selected = selected;
	return m_selected;
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB