#include "Headquarters.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"

Headquarters::Headquarters(const glm::vec3& startingPosition, const Model& model)
	: Entity(startingPosition, model),
	m_waypointPosition(startingPosition)
{}

void Headquarters::setWaypointPosition(const glm::vec3& position)
{
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