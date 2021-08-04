#pragma once

#include "Entity.h"
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

class MovingEntity : public Entity
{
protected:
	MovingEntity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType,
		int health, int shield, glm::vec3 startingRotation = glm::vec3(0.0f)) 
		: Entity(model, startingPosition, entityType, health, shield, startingRotation)
	{}

	std::vector<glm::vec3> m_movementPath;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};