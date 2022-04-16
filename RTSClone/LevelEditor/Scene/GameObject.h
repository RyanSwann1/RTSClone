#pragma once

#include "Core/AABB.h"
#include <functional>

class ShaderHandler;
struct Model;
struct GameObject
{
	GameObject(Model& model);
	GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
	GameObject(Model& model, const glm::vec3& startingPosition, const glm::vec3& rotation, const glm::vec3& scale, 
		float left, float right, float forward, float back, bool useLocalScale);
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	void setPosition(const glm::vec3& position);
	void move(const glm::vec3& position);
	void rotate(float y);
	void render(ShaderHandler& shaderHandler, bool highlight = false) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	AABB aabb;
	std::reference_wrapper<Model> model;
	bool useLocalScale;
};