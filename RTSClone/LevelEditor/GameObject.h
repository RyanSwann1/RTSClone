#pragma once

#include "AABB.h"
#include <functional>

class ShaderHandler;
struct Model;
struct GameObject
{
	GameObject(const Model& model);
	GameObject(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, bool highlight = false) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	glm::vec3 position;
	glm::vec3 rotation;
	AABB aabb;
	std::reference_wrapper<const Model> model;
};