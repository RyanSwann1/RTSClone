#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "ModelName.h"
#include "EntityType.h"

class ShaderHandler;
struct Model;
class Map;
class Entity
{
public:
	eEntityType getType() const;
	eModelName getModelName() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:	
	Entity(const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType);

	eModelName m_modelName;
	glm::vec3 m_position;
	AABB m_AABB;
	eEntityType m_type;
	bool m_selected;
};