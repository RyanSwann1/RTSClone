#include "Headquarters.h"

Headquarters::Headquarters(const glm::vec3& startingPosition, const Model& model)
	: Entity(startingPosition, model),
	m_waypointPosition(startingPosition)
{}

void Headquarters::render(ShaderHandler & shaderHandler, const Model & renderModel, const Model & waypointModel) const
{
	if (m_selected)
	{
		Entity::render(shaderHandler, waypointModel);
	}
	
	Entity::render(shaderHandler, renderModel);
}