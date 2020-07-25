#pragma once

#include "glm/glm.hpp"
#include "Rectangle2D.h" 

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
	Rectangle2D m_AABB;
};