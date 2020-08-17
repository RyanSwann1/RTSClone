#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"
#include "Map.h"
#include "ModelManager.h"

Entity::Entity(const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType)
	: m_modelName(modelName),
	m_position(),
	m_AABB(),
	m_type(entityType),
	m_selected(false)
{
	m_position = Globals::convertToMiddleGridPosition(startingPosition);
	m_AABB.reset(m_position, ModelManager::getInstance().getModel(m_modelName));
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

void Entity::render(ShaderHandler& shaderHandler, const Model& renderModel) const
{
	renderModel.render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB