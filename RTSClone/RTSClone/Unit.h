#pragma once

#include "Entity.h"
#include <vector>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

class Map;
class ShaderHandler;
class ModelManager;
class Unit : public Entity
{
public:
	Unit(const glm::vec3& startingPosition, const Model& model, Map& map);

	void moveTo(const glm::vec3& destinationPosition, const Map& map);
	void update(float deltaTime, const ModelManager& modelManager);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	glm::vec3 m_front;
	std::vector<glm::vec3> m_pathToPosition;

#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};