#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"

Unit::Unit(const glm::vec3& startingPosition, eUnitType type)
	: m_position(startingPosition),
	m_type(type)
{}

void Unit::render(ShaderHandler & shaderHandler, const Model & model) const
{
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", glm::translate(glm::mat4(1.0f), m_position));
	model.render(shaderHandler);
}
