#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "ModelName.h"

class ShaderHandler;
class SceneryGameObject : private NonCopyable
{
public:
	SceneryGameObject(eModelName modelName, const glm::vec3& position);
	SceneryGameObject(SceneryGameObject&&) noexcept;
	SceneryGameObject& operator=(SceneryGameObject&&) noexcept;
	~SceneryGameObject();

	void render(ShaderHandler& shaderHandler) const;
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	eModelName m_modelName;
	glm::vec3 m_position;
	bool m_active;
};