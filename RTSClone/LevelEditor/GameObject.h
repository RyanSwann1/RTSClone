#pragma once

#include "ModelName.h"
#include "glm/glm.hpp"

class ShaderHandler;
struct GameObject
{
	GameObject(eModelName modelName, const glm::vec3& startingPosition, const glm::vec3& startingScale);

	void render(ShaderHandler& shaderHandler) const;

	eModelName modelName;
	glm::vec3 position;
	glm::vec3 scale;
};