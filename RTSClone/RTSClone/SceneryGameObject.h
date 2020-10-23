#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "Model.h"
#include <functional>

class ShaderHandler;
class SceneryGameObject : private NonCopyable
{
public:
	SceneryGameObject(const Model& model, const glm::vec3& position);
	SceneryGameObject(SceneryGameObject&&) noexcept;
	SceneryGameObject& operator=(SceneryGameObject&&) noexcept;
	~SceneryGameObject();

	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::reference_wrapper<const Model> m_model;
	glm::vec3 m_position;
	bool m_active;
};