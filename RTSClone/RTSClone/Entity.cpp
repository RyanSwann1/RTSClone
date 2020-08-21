#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"
#include "Map.h"
#include "ModelManager.h"

Entity::Entity(int ID, const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType)
	: m_position(0.0f, 0.0f, 0.0f),
	m_AABB(),
	m_ID(ID),
	m_modelName(modelName),
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
		m_position = startingPosition;
		break;
	default: 
		assert(false);
	}
	
	m_AABB.reset(m_position, ModelManager::getInstance().getModel(m_modelName));
}

Entity::Entity(Entity&& orig) noexcept
	: m_position(orig.m_position),
	m_AABB(std::move(orig.m_AABB)),
	m_ID(orig.m_ID),
	m_modelName(orig.m_modelName),
	m_type(orig.m_type),
	m_selected(orig.m_selected)
{
	orig.m_ID = Globals::INVALID_ENTITY_ID;
}

Entity& Entity::operator=(Entity&& orig) noexcept
{
	m_position = orig.m_position;
	m_AABB = std::move(orig.m_AABB);
	m_ID = orig.m_ID;
	m_modelName = orig.m_modelName;
	m_type = orig.m_type;
	m_selected = orig.m_selected;

	orig.m_ID = Globals::INVALID_ENTITY_ID;
	return *this;
}

int Entity::getID() const
{
	return m_ID;
}

eEntityType Entity::getType() const
{
	return m_type;
}

eModelName Entity::getModelName() const
{
	return m_modelName;
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

void Entity::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(m_modelName).render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB