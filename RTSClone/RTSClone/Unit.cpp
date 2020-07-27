#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "PathFinding.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;

	glm::vec3 moveTowards(const glm::vec3& currentPosition, const glm::vec3& targetPosition, float maxDistanceDelta)
	{
		float magnitude = glm::distance(targetPosition, currentPosition);
		if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
		{
			return targetPosition;
		}

		return currentPosition + glm::vec3(targetPosition - currentPosition) / magnitude * maxDistanceDelta;
	}
}

Unit::Unit(const glm::vec3& startingPosition)
	: m_position(startingPosition), 
	m_AABB(startingPosition, 5.0f),
	m_selected(false),
	m_pathToPosition()
{}

const AABB& Unit::getAABB() const
{
	return m_AABB;
}

bool Unit::isSelected() const
{
	return m_selected;
}

void Unit::setSelected(bool selected)
{
	m_selected = selected;
}

void Unit::moveTo(const glm::vec3& destinationPosition)
{
	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(m_position, destinationPosition, m_pathToPosition);
}

void Unit::update(float deltaTime)
{
	if (!m_pathToPosition.empty())
	{
		glm::vec3 newPosition = moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_position = newPosition;
		if (m_position == m_pathToPosition.back())
		{
			m_pathToPosition.pop_back();
		}
	}
}

void Unit::render(ShaderHandler& shaderHandler, const Model& renderModel) const
{
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, m_position);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
	renderModel.render(shaderHandler);
}