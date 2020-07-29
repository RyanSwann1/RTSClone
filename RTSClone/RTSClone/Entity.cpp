#include "Entity.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "ShaderHandler.h"

Entity::Entity(const glm::vec3& startingPosition, const Model& model)
	: m_position(startingPosition),
	m_AABB(startingPosition, model),
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
	glm::mat4 model = glm::mat4(1.0f);
	if (renderModel.renderFromCentrePosition)
	{
		glm::vec3 position(m_position.x + (m_AABB.m_right - m_AABB.m_left) / 2.0f, m_position.y, m_position.z - (m_AABB.m_forward - m_AABB.m_back) / 2.0f);
		//glm::vec3 position = m_position;
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::translate(model, position);
	}
	else
	{
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::translate(model, m_position);
	}

	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
	renderModel.render(shaderHandler, m_selected);
}

#ifdef RENDER_AABB
void Entity::renderAABB(ShaderHandler& shaderHandler)
{
	m_AABB.render(shaderHandler);
}
#endif // RENDER_AABB