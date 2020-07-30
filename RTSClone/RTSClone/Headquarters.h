#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

struct Camera;
class Headquarters : public Entity
{
public:
	Headquarters(const glm::vec3& startingPosition, const Model& model, Map& map);

	glm::vec3 getUnitSpawnPosition() const;
	void setWaypointPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, const Model& renderModel, const Model& waypointModel) const;

private:
	glm::vec3 m_waypointPosition;
};