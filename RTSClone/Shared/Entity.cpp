#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"
#include "ModelManager.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"

#ifdef GAME
Entity::Entity(const glm::vec3& startingPosition, eEntityType entityType, int health)
	: m_position(0.0f, 0.0f, 0.0f),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_health(health),
	m_type(entityType),
	m_selected(false)
{
	switch (m_type)
	{
	case eEntityType::Barracks:
	case eEntityType::HQ:
	case eEntityType::Mineral:
	case eEntityType::SupplyDepot:
	case eEntityType::Unit:
		m_position = Globals::convertToMiddleGridPosition(startingPosition);
		break;
	case eEntityType::Worker:
	case eEntityType::Projectile:
		m_position = startingPosition;
		break;
	default:
		assert(false);
	}

	m_AABB.reset(m_position, ModelManager::getInstance().getModel(m_type));
}
#endif // GAME

#ifdef LEVEL_EDITOR
Entity::Entity()
	: m_position(),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_modelName(),
	m_selected(false)
{}

Entity::Entity(eModelName modelName, const glm::vec3& startingPosition)
	: m_position(startingPosition),
	m_AABB(),
	m_ID(UniqueEntityIDDistributer::getInstance().getUniqueEntityID()),
	m_modelName(modelName),
	m_selected(false)
{
	m_AABB.reset(m_position, ModelManager::getInstance().getModel(m_modelName));
}

glm::vec3& Entity::getPosition()
{
	return m_position;
}

void Entity::setModelName(eModelName modelName)
{
	m_modelName = modelName;
}

void Entity::setPosition(const glm::vec3 & position)
{
	m_position = position;
}

eModelName Entity::getModelName() const
{
	return m_modelName;
}
void Entity::resetAABB()
{
	m_AABB.reset(m_position, ModelManager::getInstance().getModel(m_modelName));
}
#endif // LEVEL_EDITOR

Entity::Entity(Entity&& orig) noexcept
	: m_position(orig.m_position),
	m_AABB(std::move(orig.m_AABB)),
	m_ID(orig.m_ID),
	m_selected(orig.m_selected)
{
#ifdef GAME
	m_health = orig.m_health;
	m_type = orig.m_type;
#endif // GAME
#ifdef LEVEL_EDITOR
	m_modelName = orig.m_modelName;
#endif // LEVEL_EDITOR

	orig.m_ID = Globals::INVALID_ENTITY_ID;
}

Entity& Entity::operator=(Entity&& orig) noexcept
{
	m_position = orig.m_position;
	m_AABB = std::move(orig.m_AABB);
	m_ID = orig.m_ID;
#ifdef GAME
	m_health = orig.m_health;
	m_type = orig.m_type;
#endif // GAME
#ifdef LEVEL_EDITOR
	m_modelName = orig.m_modelName;
#endif // LEVEL_EDITOR
	m_selected = orig.m_selected;

	orig.m_ID = Globals::INVALID_ENTITY_ID;
	return *this;
}

int Entity::getID() const
{
	return m_ID;
}

#ifdef GAME
eEntityType Entity::getEntityType() const
{
	return m_type;
}

int Entity::getHealth() const
{
	return m_health;
}

void Entity::reduceHealth(int damage)
{
	m_health -= damage;
}

bool Entity::isDead() const
{
	return m_health == 0;
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
#ifdef GAME
	ModelManager::getInstance().getModel(m_type).render(shaderHandler, *this);
#endif // GAME

#ifdef LEVEL_EDITOR
	ModelManager::getInstance().getModel(m_modelName).render(shaderHandler, *this);
#endif // LEVEL_EDITOR
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB