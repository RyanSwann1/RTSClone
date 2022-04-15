#pragma once

#include "glm/glm.hpp"
#include "Core/AABB.h"
#include <functional>

struct Model;
class ShaderHandler;
struct SceneryGameObject
{
	SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation, 
		const glm::vec3& scale, float left, float right, float forward, float back);

	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	std::reference_wrapper<const Model> model;
	AABB AABB;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};