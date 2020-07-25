#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

Unit::Unit(const glm::vec3& startingPosition)
	: m_position(startingPosition), 
	m_AABB(startingPosition, 1.0f)
{}

void Unit::render(ShaderHandler& shaderHandler, const Model& renderModel) const
{
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, m_position);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
	renderModel.render(shaderHandler);
}