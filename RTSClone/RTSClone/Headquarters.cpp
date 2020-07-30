#include "Headquarters.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"

namespace
{
	
}

Headquarters::Headquarters(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Entity(startingPosition, model, eEntityType::Building, map),
	m_waypointPosition(startingPosition)
{}

glm::vec3 Headquarters::getUnitSpawnPosition() const
{
	assert(m_selected);


	return glm::vec3();
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