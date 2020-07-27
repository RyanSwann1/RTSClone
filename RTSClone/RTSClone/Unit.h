#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include <vector>

enum class eUnitType
{
	Default = 0
};

class ShaderHandler;
struct Model;
struct Unit
{
	Unit(const glm::vec3& startingPosition);

	void moveTo(const glm::vec3& destinationPosition);

	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

	glm::vec3 m_position;
	AABB m_AABB;
	bool m_selected;
	std::vector<glm::vec3> m_pathToPosition;
};