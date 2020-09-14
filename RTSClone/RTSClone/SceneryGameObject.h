#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "ModelName.h"

class ShaderHandler;
struct SceneryGameObject : private NonCopyable
{
	SceneryGameObject(eModelName modelName, const glm::vec3& position);
	SceneryGameObject(SceneryGameObject&&) noexcept;
	SceneryGameObject& operator=(SceneryGameObject&&) noexcept;

	void render(ShaderHandler& shaderHandler) const;

	eModelName modelName;
	glm::vec3 position;
};