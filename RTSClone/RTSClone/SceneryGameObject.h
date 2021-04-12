#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "ActiveStatus.h"
#include <functional>

class Model;
class ShaderHandler;
class SceneryGameObject
{
public:
	SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation);
	SceneryGameObject(const SceneryGameObject&) = delete;
	SceneryGameObject& operator=(const SceneryGameObject&) = delete;
	SceneryGameObject(SceneryGameObject&&) = default;
	SceneryGameObject& operator=(SceneryGameObject&&) = default;
	~SceneryGameObject();

	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::reference_wrapper<const Model> m_model;
	AABB m_AABB;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	ActiveStatus m_active;
};