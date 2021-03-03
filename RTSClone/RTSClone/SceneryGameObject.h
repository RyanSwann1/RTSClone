#pragma once

#include "glm/glm.hpp"
#include "Model.h"
#include "AABB.h"
#include <functional>

class ShaderHandler;
class SceneryGameObject
{
public:
	SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation);
	SceneryGameObject(const SceneryGameObject&) = delete;
	SceneryGameObject& operator=(const SceneryGameObject&) = delete;
	SceneryGameObject(SceneryGameObject&&) noexcept;
	SceneryGameObject& operator=(SceneryGameObject&&) noexcept;
	~SceneryGameObject();

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
	bool m_active;
};