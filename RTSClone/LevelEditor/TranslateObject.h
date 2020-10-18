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

	void render(ShaderHandler& shaderHandler) const;

private:
	glm::vec3 m_centerPosition;
	Mesh m_mesh;
	glm::vec3 m_xPosition;
	AABB m_xAABB;
	glm::vec3 m_yPosition;
	AABB m_yAABB;
	glm::vec3 m_zPosition;
	AABB m_zAABB;
};