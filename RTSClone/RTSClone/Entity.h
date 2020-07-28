#pragma once

#include "glm/glm.hpp"
#include "AABB.h"

class ShaderHandler;
struct Model;
class Entity
{
public:
	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	virtual void update(float deltaTime) {}
	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Entity(const glm::vec3& startingPosition);

	glm::vec3 m_position;
	AABB m_AABB;
	bool m_selected;
};