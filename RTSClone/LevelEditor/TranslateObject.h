#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include "AABB.h"
#include <glm/glm.hpp>

class ShaderHandler;
class TranslateObject : private NonCopyable, private NonMovable
{
public:
	TranslateObject();

	bool isColliding(const glm::vec3& mouseToGroundPosition) const;
	void setActive(bool active);
	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	bool m_active;
	glm::vec3 m_centerPosition;
	glm::vec3 m_xPosition;
	AABB m_xAABB;
	glm::vec3 m_zPosition;
	AABB m_zAABB;
};