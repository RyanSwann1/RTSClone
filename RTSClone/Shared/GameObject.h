#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "ModelName.h"
#include "glm/glm.hpp"
#include "AABB.h"

class ShaderHandler;
struct GameObject
{
	GameObject(eModelName modelName, const glm::vec3& startingPosition);

	void render(ShaderHandler& shaderHandler) const;
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	eModelName modelName;
	glm::vec3 position;
	AABB AABB;
#ifdef LEVEL_EDITOR
	bool selected;
#endif // LEVEL_EDITOR
};