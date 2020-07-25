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
	Unit(const glm::vec3& startingPosition, eUnitType type);

	void render(ShaderHandler& shaderHandler, const Model& model) const;

	glm::vec3 m_position;
	eUnitType m_type;
	Rectangle2D m_AABB;
};