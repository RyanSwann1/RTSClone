#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include <vector>

#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

class ShaderHandler;
struct Model;
class Unit
{
public:
	Unit(const glm::vec3& startingPosition);

	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void moveTo(const glm::vec3& destinationPosition);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;
#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	glm::vec3 m_position;
	glm::vec3 m_front;
	AABB m_AABB;
	bool m_selected;
	std::vector<glm::vec3> m_pathToPosition;

#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};