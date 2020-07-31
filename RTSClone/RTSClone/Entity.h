#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "ModelName.h"

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
	
	eModelName getModelName() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void render(ShaderHandler& shaderHandler, const Model& renderModel) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	eModelName m_modelName;
	glm::vec3 m_position;
	AABB m_AABB;
	eEntityType m_type;
	bool m_selected;
};