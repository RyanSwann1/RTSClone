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
	const float DEFAULT_HEALTH_BAR_HEIGHT = 10.0f;
	const float WORKER_HEALTH_BAR_YOFFSET = 35.0f;
	const float UNIT_HEALTH_BAR_YOFFSET = 50.0f;
	const float HQ_HEALTH_BAR_YOFFSET = 225.0f;
	const float SUPPLY_DEPOT_HEALTH_BAR_YOFFSET = 85.0f;
	const float BARRACKS_HEALTH_BAR_YOFFSET = 85.0f;
	const float TURRET_HEALTH_BAR_YOFFSET = 60.0f;
	const float LABORATORY_HEALTH_BAR_YOFFSET = 130.0f;

	const float DEFAULT_SHIELD_BAR_HEIGHT = 10.0f;
	const float WORKER_SHIELD_BAR_YOFFSET = 45.0f;
	const float UNIT_SHIELD_BAR_YOFFSET = 60.0f;
	const float HQ_SHIELD_BAR_YOFFSET = 235.0f;
	const float SUPPLY_DEPOT_SHIELD_BAR_YOFFSET = 95.0f;
	const float BARRACKS_SHIELD_BAR_YOFFSET = 95.0f;
	const float TURRET_SHIELD_BAR_YOFFSET = 70.0f;
	const float LABORATORY_SHIELD_BAR_YOFFSET = 140.0f;

	const float SHIELD_REPLENISH_TIMER_EXPIRATION = 15.0f;
}

Entity::Entity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
	int health, int shield, glm::vec3 startingRotation)
	: m_statbarSprite(),
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
	case eEntityType::HQ:
	case eEntityType::SupplyDepot:
	case eEntityType::Unit:
	case eEntityType::Turret:
	case eEntityType::Laboratory:
		m_position = Globals::convertToMiddleGridPosition(startingPosition);
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

void Entity::reduceHealth(const TakeDamageEvent& gameEvent)
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
		float width = 0.0f;
		float yOffset = 0.0f;
		switch (getEntityType())
		{
		case eEntityType::Unit:
			width = Globals::UNIT_STAT_BAR_WIDTH;
			yOffset = UNIT_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::Worker:
			width = Globals::WORKER_STAT_BAR_WIDTH;
			yOffset = WORKER_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::HQ:
			width = Globals::HQ_STAT_BAR_WIDTH;
			yOffset = HQ_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::SupplyDepot:
			width = Globals::SUPPLY_DEPOT_STAT_BAR_WIDTH;
			yOffset = SUPPLY_DEPOT_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::Barracks:
			width = Globals::BARRACKS_STAT_BAR_WIDTH;
			yOffset = BARRACKS_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::Turret:
			width = Globals::TURRET_STAT_BAR_WIDTH;
			yOffset = TURRET_HEALTH_BAR_YOFFSET;
			break;
		case eEntityType::Laboratory:
			width = Globals::LABORATORY_STAT_BAR_WIDTH;
			yOffset = LABORATORY_HEALTH_BAR_YOFFSET;
			break;
		default:
			assert(false);
		}
		
		m_statbarSprite.render(m_position, windowSize, width, width, DEFAULT_HEALTH_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::BACKGROUND_BAR_COLOR);
		
		float currentHealth = static_cast<float>(m_health) / static_cast<float>(m_maximumHealth);
		m_statbarSprite.render(m_position, windowSize, width, width * currentHealth, DEFAULT_HEALTH_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::HEALTH_BAR_COLOR);
	}
}

void Entity::renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (isSelected() && m_shield > 0)
	{
		float width = 0.0f;
		float yOffset = 0.0f;
		switch (getEntityType())
		{
		case eEntityType::Unit:
			width = Globals::UNIT_STAT_BAR_WIDTH;
			yOffset = UNIT_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::Worker:
			width = Globals::WORKER_STAT_BAR_WIDTH;
			yOffset = WORKER_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::HQ:
			width = Globals::HQ_STAT_BAR_WIDTH;
			yOffset = HQ_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::SupplyDepot:
			width = Globals::SUPPLY_DEPOT_STAT_BAR_WIDTH;
			yOffset = SUPPLY_DEPOT_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::Barracks:
			width = Globals::BARRACKS_STAT_BAR_WIDTH;
			yOffset = BARRACKS_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::Turret:
			width = Globals::TURRET_STAT_BAR_WIDTH;
			yOffset = TURRET_SHIELD_BAR_YOFFSET;
			break;
		case eEntityType::Laboratory:
			width = Globals::LABORATORY_STAT_BAR_WIDTH;
			yOffset = LABORATORY_SHIELD_BAR_YOFFSET;
			break;
		default:
			assert(false);
		}

		m_statbarSprite.render(m_position, windowSize, width, width, DEFAULT_SHIELD_BAR_HEIGHT, yOffset,
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

void Entity::setSelected(bool selected)
{
	m_selected = selected;
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB