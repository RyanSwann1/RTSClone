#include "Entity.h"
#include "Model.h"
#include "ShaderHandler.h"

Entity::Entity(const glm::vec3& startingPosition, const Model& model)
	: m_position(startingPosition),
	m_AABB(startingPosition, model),
	m_selected(false)
{}

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