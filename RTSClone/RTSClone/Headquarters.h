#pragma once

#include "Entity.h"

class Headquarters : public Entity
{
public:
	Headquarters(const glm::vec3& startingPosition, const Model& model);


	void render(ShaderHandler& shaderHandler, const Model& renderModel, const Model& waypointModel) const;

private:
	glm::vec3 m_waypointPosition;
};