#include "Entity.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "ShaderHandler.h"

Entity::Entity(const glm::vec3& startingPosition)
	: m_position(startingPosition),
	m_AABB(startingPosition, 5.0f),
	m_selected(false)
{}

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
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, m_position);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
	renderModel.render(shaderHandler, m_selected);
	renderModel.render(shaderHandler, m_selected);
}

void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
