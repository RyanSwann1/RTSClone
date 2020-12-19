#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"
#include "ModelManager.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"
#include "FactionController.h"
#ifdef GAME
#include "Faction.h"
#include "GameEvent.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"
#endif // GAME

#ifdef GAME
namespace
{
	const float WORKER_HEALTHBAR_WIDTH = 60.0f;
	const float WORKER_HEALTHBAR_YOFFSET = 25.0f;
	const float UNIT_HEALTHBAR_WIDTH = 75.0f;
	const float UNIT_HEALTHBAR_YOFFSET = 50.0f;
	const float HQ_HEALTHBAR_WIDTH = 150.0f;
	const float HQ_HEALTHBAR_YOFFSET = 225.0f;
	const float SUPPLY_DEPOT_HEALTHBAR_WIDTH = 100.0f;
	const float SUPPLY_DEPOT_HEALTHBAR_YOFFSET = 85.0f;
	const float BARRACKS_HEALTHBAR_WIDTH = 100.0f;
	const float BARRACKS_HEALTHBAR_YOFFSET = 85.0f;
	const float TURRET_HEALTHBAR_WIDTH = 100.0f;
	const float TURRET_HEALTHBAR_YOFFSET = 60.0f;
	const float DEFAULT_HEALTH_BAR_HEIGHT = 10.0f;

	const float WORKER_SHIELD_BAR_WIDTH = 60.0f;
	const float WORKER_SHIELD_BAR_YOFFSET = 35.0f;
	const float UNIT_SHIELD_BAR_WIDTH = 75.0f;
	const float UNIT_SHIELD_BAR_YOFFSET = 60.0f;
	const float HQ_SHIELD_BAR_WIDTH = 150.0f;
	const float HQ_SHIELD_BAR_YOFFSET = 235.0f;
	const float SUPPLY_DEPOT_SHIELD_BAR_WIDTH = 100.0f;
	const float SUPPLY_DEPOT_SHIELD_BAR_YOFFSET = 95.0f;
	const float BARRACKS_SHIELD_BAR_WIDTH = 100.0f;
	const float BARRACKS_SHIELD_BAR_YOFFSET = 95.0f;
	const float TURRET_SHIELD_BAR_WIDTH = 100.0f;
	const float TURRET_SHIELD_BAR_YOFFSET = 70.0f;
	const float DEFAULT_SHIELD_BAR_HEIGHT = 10.0f;

	const glm::vec3 HEALTH_BAR_COLOR = { 0.0f, 0.8f, 0.0f };
	const glm::vec3 SHIELD_BAR_COLOR = { 0.0f, 1.0f, 1.0f };
}
#endif // GAME

#ifdef GAME
Entity::Entity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
	int health, glm::vec3 startingRotation)
	: m_position(startingPosition),
	m_rotation(startingRotation),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_maximumShield(0),
	m_shield(m_maximumShield),
	m_maximumHealth(health),
	m_health(health),
	m_type(entityType),
	m_healthbarSprite(),
	m_model(model),
	m_selected(false)
{
	switch (m_type)
	{
	case eEntityType::Barracks:
	case eEntityType::HQ:
	case eEntityType::Mineral:
	case eEntityType::SupplyDepot:
	case eEntityType::Unit:
	case eEntityType::Turret:
	case eEntityType::Laboratory:
		m_position = Globals::convertToMiddleGridPosition(startingPosition);
		break;
	case eEntityType::Worker:
	case eEntityType::Projectile:
		break;
	default:
		assert(false);
	}

	m_AABB.reset(m_position, m_model);
}
#endif // GAME

#ifdef LEVEL_EDITOR
Entity::Entity(const Model& model)
	: m_position(),
	m_rotation(),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_model(model),
	m_selected(false)
{}

Entity::Entity(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: m_position(startingPosition),
	m_rotation(startingRotation),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_model(model),
	m_selected(false)
{
	m_AABB.reset(m_position, m_model);
}

glm::vec3& Entity::getRotation()
{
	return m_rotation;
}

glm::vec3& Entity::getPosition()
{
	return m_position;
}

const Model& Entity::getModel() const
{
	return m_model;
}

void Entity::setPosition(const glm::vec3 & position)
{
	m_position = position;
}

void Entity::setRotation(const glm::vec3 rotation)
{
	m_rotation = rotation;
}

void Entity::resetAABB()
{
	m_AABB.reset(m_position, m_model);
}
#endif // LEVEL_EDITOR

Entity::~Entity()
{}

Entity::Entity(Entity&& rhs) noexcept
	: m_position(rhs.m_position),
	m_rotation(rhs.m_rotation),
	m_AABB(std::move(rhs.m_AABB)),
	m_ID(rhs.m_ID),
	m_model(rhs.m_model),
	m_selected(rhs.m_selected)
{
#ifdef GAME
	m_maximumShield = rhs.m_maximumShield;
	m_shield = rhs.m_shield;
	m_maximumHealth = rhs.m_maximumHealth;
	m_health = rhs.m_health;
	m_type = rhs.m_type;
	m_healthbarSprite = std::move(rhs.m_healthbarSprite);
#endif // GAME

	rhs.m_ID = Globals::INVALID_ENTITY_ID;
}

Entity& Entity::operator=(Entity&& rhs) noexcept
{
	m_position = rhs.m_position;
	m_rotation = rhs.m_rotation;
	m_AABB = std::move(rhs.m_AABB);
	m_ID = rhs.m_ID;
	m_model = std::move(rhs.m_model);
	m_selected = rhs.m_selected;

#ifdef GAME
	m_maximumShield = rhs.m_maximumShield;
	m_shield = rhs.m_shield;
	m_maximumHealth = rhs.m_maximumHealth;
	m_health = rhs.m_health;
	m_type = rhs.m_type;
#endif // GAME

	rhs.m_ID = Globals::INVALID_ENTITY_ID;
	return *this;
}

int Entity::getID() const
{
	return m_ID;
}

const glm::vec3& Entity::getRotation() const
{
	return m_rotation;
}

#ifdef GAME
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

void Entity::increaseShield(const Faction& owningFaction)
{
	m_maximumShield = owningFaction.getCurrentShieldAmount();
	++m_shield;
	assert(m_shield <= m_maximumShield);
}

void Entity::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	m_model.get().render(shaderHandler, *this, owningFactionController);
}

void Entity::renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (isSelected())
	{
		glm::vec4 positionNDC = camera.getProjection(glm::ivec2(windowSize.x, windowSize.y)) * camera.getView() * glm::vec4(m_position, 1.0f);
		positionNDC /= positionNDC.w;
		float width = 0.0f;
		float height = DEFAULT_HEALTH_BAR_HEIGHT / windowSize.y * 2.0f;
		float yOffset = 0.0f;
		float currentHealth = static_cast<float>(m_health) / static_cast<float>(m_maximumHealth);
		switch (getEntityType())
		{
		case eEntityType::Unit:
			width = UNIT_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = UNIT_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Worker:
			width = WORKER_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = WORKER_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::HQ:
			width = HQ_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = HQ_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::SupplyDepot:
			width = SUPPLY_DEPOT_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = SUPPLY_DEPOT_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Barracks:
			width = BARRACKS_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = BARRACKS_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Turret:
			width = TURRET_HEALTHBAR_WIDTH * currentHealth / windowSize.x * 2.0f;
			yOffset = TURRET_HEALTHBAR_YOFFSET / windowSize.y * 2.0f;
			break;
		default:
			assert(false);
		}
		
		shaderHandler.setUniformVec3(eShaderType::HealthBar, "uMaterialColor", HEALTH_BAR_COLOR);
		m_healthbarSprite.render({ positionNDC.x, positionNDC.y }, windowSize, width, height, yOffset);
	}
}
void Entity::renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (isSelected() && m_shield > 0)
	{
		glm::vec4 positionNDC = camera.getProjection(glm::ivec2(windowSize.x, windowSize.y)) * camera.getView() * glm::vec4(m_position, 1.0f);
		positionNDC /= positionNDC.w;
		float width = 0.0f;
		float height = DEFAULT_SHIELD_BAR_HEIGHT / windowSize.y * 2.0f;
		float yOffset = 0.0f;
		switch (getEntityType())
		{
		case eEntityType::Unit:
			width = UNIT_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = UNIT_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Worker:
			width = WORKER_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = WORKER_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::HQ:
			width = HQ_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = HQ_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::SupplyDepot:
			width = SUPPLY_DEPOT_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = SUPPLY_DEPOT_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Barracks:
			width = BARRACKS_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = BARRACKS_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		case eEntityType::Turret:
			width = TURRET_SHIELD_BAR_WIDTH / windowSize.x * 2.0f;
			yOffset = TURRET_SHIELD_BAR_YOFFSET / windowSize.y * 2.0f;
			break;
		default:
			assert(false);
		}

		shaderHandler.setUniformVec3(eShaderType::HealthBar, "uMaterialColor", SHIELD_BAR_COLOR);
		m_healthbarSprite.render({ positionNDC.x, positionNDC.y }, windowSize, width, height, yOffset);
	}
}
#endif // GAME

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

void Entity::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB