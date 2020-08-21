#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "ModelName.h"
#include "EntityType.h"

class ShaderHandler;
struct Model;
class Map;
class Entity : protected NonCopyable
{
public:
	Entity(Entity&&) noexcept;
	Entity& operator=(Entity&&) noexcept;

	int getID() const;
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
	Entity(int ID, const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType);
	glm::vec3 m_position;
	AABB m_AABB;
	
private:
	int m_ID;
	eModelName m_modelName;
	eEntityType m_type;
	bool m_selected;
};