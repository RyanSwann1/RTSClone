#pragma once

#include "glm/glm.hpp"
#include "AABB.h"

enum class eEntityType
{
	Unit = 0,
	Building,
	Mineral
};

class ShaderHandler;
struct Model;
class Map;
class Entity
{
public:
	Entity(const glm::vec3& startingPosition, const Model& model, eEntityType entityType, Map& map);
	
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void update(float deltaTime) {}
	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	glm::vec3 m_position;
	AABB m_AABB;
	eEntityType m_type;
	bool m_selected;
};