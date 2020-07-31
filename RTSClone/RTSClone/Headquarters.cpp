#include "Headquarters.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"

namespace
{
	glm::vec3 getSpawnPosition(const AABB& AABB, const glm::vec3& direction, const glm::vec3& startingPosition)
	{
		glm::vec3 position = startingPosition;
		float distance = 1;
		while (AABB.contains(position))
		{
			position = position + direction * distance;
			++distance;
		}
		
		return position;
	}
}

Headquarters::Headquarters(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Entity(startingPosition, model, eEntityType::Building, map),
	m_waypointPosition(startingPosition)
{}

glm::vec3 Headquarters::getUnitSpawnPosition() const
{
	assert(m_selected);
	glm::vec3 unitSpawnPosition;
	if (m_waypointPosition != m_position)
	{
		unitSpawnPosition = getSpawnPosition(m_AABB, glm::normalize(m_waypointPosition - m_position), m_position);
	}
	else
	{
		unitSpawnPosition = getSpawnPosition(m_AABB, 
			glm::normalize(glm::vec3(Globals::getRandomNumber(-1.0f, 1.0f), Globals::GROUND_HEIGHT, Globals::getRandomNumber(-1.0f, 1.0f))), 
			m_position);
	}

	return unitSpawnPosition;
}

void Headquarters::setWaypointPosition(const glm::vec3& position)
{
	assert(m_selected);
	if (Globals::isPositionInMapBounds(position))
	{
		if (m_AABB.contains(position))
		{
			m_waypointPosition = m_position;
		}
		else
		{
			m_waypointPosition = position;
		}
	}
}

void Headquarters::render(ShaderHandler & shaderHandler, const Model & renderModel, const Model & waypointModel) const
{
	if (m_selected && m_waypointPosition != m_position)
	{
		waypointModel.render(shaderHandler, m_waypointPosition);
	}
	
	renderModel.render(shaderHandler, *this);
}