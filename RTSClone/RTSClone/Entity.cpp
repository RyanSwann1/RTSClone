#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"
#include "ModelManager.h"
#include "Globals.h"
#include "UniqueIDGenerator.h"
#include "FactionController.h"
#include "Faction.h"
#include "GameEvents.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"

namespace
{
	const float DEFAULT_STAT_BAR_HEIGHT = 10.0f;

	const float WORKER_HEALTH_BAR_YOFFSET = 35.0f;
	const float UNIT_HEALTH_BAR_YOFFSET = 50.0f;
	const float HQ_HEALTH_BAR_YOFFSET = 225.0f;
	const float SUPPLY_DEPOT_HEALTH_BAR_YOFFSET = 85.0f;
	const float BARRACKS_HEALTH_BAR_YOFFSET = 85.0f;
	const float TURRET_HEALTH_BAR_YOFFSET = 60.0f;
	const float LABORATORY_HEALTH_BAR_YOFFSET = 130.0f;

	const float UNIT_SHIELD_BAR_YOFFSET = 60.0f;
	const float WORKER_SHIELD_BAR_YOFFSET = 45.0f;
	const float HQ_SHIELD_BAR_YOFFSET = 235.0f;
	const float SUPPLY_DEPOT_SHIELD_BAR_YOFFSET = 95.0f;
	const float BARRACKS_SHIELD_BAR_YOFFSET = 95.0f;
	const float TURRET_SHIELD_BAR_YOFFSET = 70.0f;
	const float LABORATORY_SHIELD_BAR_YOFFSET = 140.0f;

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

//EntityStatus
EntityStatus::EntityStatus()
	: active(true)
{}

EntityStatus::EntityStatus(EntityStatus&& rhs) noexcept
	: active(rhs.active)
{
	assert(false);
	rhs.active = false;
}

EntityStatus& EntityStatus::operator=(EntityStatus&& rhs) noexcept
{
	assert(false);
	active = rhs.active;
	rhs.active = false;

	return *this;
}

bool EntityStatus::isActive() const
{
	return active;
}

//Entity
Entity::Entity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
	int health, int shield, glm::vec3 startingRotation)
	: m_status(),
	m_statbarSprite(),
	m_position(startingPosition),
	m_rotation(startingRotation),
	m_AABB(),
	m_ID(UniqueIDGenerator::getInstance().getUniqueID()),
	m_maximumShield(shield),
	m_shield(m_maximumShield),
	m_maximumHealth(health),
	m_health(m_maximumHealth),
	m_type(entityType),
	m_shieldReplenishTimer(SHIELD_REPLENISH_TIMER_EXPIRATION, false),
	m_model(model),
	m_selected(false)
{
	switch (m_type)
	{
	case eEntityType::Barracks:
	case eEntityType::Headquarters:
	case eEntityType::SupplyDepot:
	case eEntityType::Unit:
	case eEntityType::Turret:
	case eEntityType::Laboratory:
		m_position = Globals::convertToNodePosition(m_position);
		m_position = Globals::convertToMiddleGridPosition(m_position);
		break;
	case eEntityType::Worker:
		break;
	default:
		assert(false);
	}

	m_AABB.reset(m_position, m_model);
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

Entity::~Entity()
{}

int Entity::getID() const
{
	return m_ID;
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

void Entity::takeDamage(const TakeDamageEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
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

void Entity::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	switch (owningFactionController)
	{
	case eFactionController::Player:
		m_model.get().render(shaderHandler, owningFactionController, m_position, m_rotation, m_selected);
		break;
	case eFactionController::AI_1:
	case eFactionController::AI_2:
	case eFactionController::AI_3:
		m_model.get().render(shaderHandler, owningFactionController, m_position, m_rotation, false);
		break;
	default:
		assert(false);
	}
}

void Entity::renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (isSelected())
	{
		float width = Globals::ENTITIES_STAT_BAR_WIDTH[static_cast<int>(getEntityType())];
		float yOffset = ENTITIES_YOFFSET_HEALTH[static_cast<int>(getEntityType())];
		
		m_statbarSprite.render(m_position, windowSize, width, width, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::BACKGROUND_BAR_COLOR);
		
		float currentHealth = static_cast<float>(m_health) / static_cast<float>(m_maximumHealth);
		m_statbarSprite.render(m_position, windowSize, width, width * currentHealth, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::HEALTH_BAR_COLOR);
	}
}

void Entity::renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (isSelected() && m_shield > 0)
	{
		float width = Globals::ENTITIES_STAT_BAR_WIDTH[static_cast<int>(getEntityType())];
		float yOffset = ENTITIES_YOFFSET_SHIELD[static_cast<int>(getEntityType())];

		m_statbarSprite.render(m_position, windowSize, width, width, DEFAULT_STAT_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::SHIELD_BAR_COLOR);
	}
}

void Entity::setPosition(const glm::vec3& position)
{
	m_position = position;
	m_AABB.update(position);
}

const glm::vec3& Entity::getPosition() const
{
	return m_position;
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

