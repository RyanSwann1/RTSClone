#pragma once

#include "Entity.h"
#include <deque>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

class MovingEntity : public Entity
{
public:
	void addToDestinationQueue(const glm::vec3& position);
	void clearDestinationQueue();
#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	MovingEntity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType,
		int health, int shield, glm::vec3 startingRotation = glm::vec3(0.0f));

	std::vector<glm::vec3> m_movementPath;
	std::deque<glm::vec3> m_destinationQueue;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};