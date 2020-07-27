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
class Unit
{
public:
	Unit(const glm::vec3& startingPosition);

	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void moveTo(const glm::vec3& destinationPosition);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

private:
	glm::vec3 m_position;
	AABB m_AABB;
	bool m_selected;
	std::vector<glm::vec3> m_pathToPosition;
};