#pragma once

#include "glm/glm.hpp"
#include "AABB.h"

enum class eUnitType
{
	Default = 0
};

class ShaderHandler;
struct Model;
struct Unit
{
	Unit(const glm::vec3& startingPosition);

	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

	glm::vec3 m_position;
	AABB m_AABB;
};