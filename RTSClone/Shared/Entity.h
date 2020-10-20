#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "AABB.h"
#ifdef LEVEL_EDITOR
#include "ModelName.h"
#endif // LEVEL_EDITOR
#ifdef GAME
#include "EntityType.h"
#endif // GAME

struct Model;
class ShaderHandler;
class Map;
class Entity : private NonCopyable
{
public:
	Entity(Entity&&) noexcept;
	Entity& operator=(Entity&&) noexcept;

#ifdef LEVEL_EDITOR
	Entity();
	Entity(eModelName modelName, const glm::vec3& startingPosition);
	glm::vec3& getPosition();
	void setModelName(eModelName modelName);
	void setPosition(const glm::vec3& position);
	eModelName getModelName() const;

	void resetAABB();
#endif // LEVEL_EDITOR
#ifdef GAME
	eEntityType getEntityType() const;
	int getMaximumHealth() const;
	int getHealth() const;
	void reduceHealth(int damage);
	bool isDead() const;
	void repair();
#endif // GAME
	int getID() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	bool isSelected() const;
	
	void setSelected(bool selected);
	void render(ShaderHandler& shaderHandler) const;
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:	
#ifdef GAME
	Entity(const glm::vec3& startingPosition, eEntityType entityType, int health = 0);
#endif // GAME
	glm::vec3 m_position;
	AABB m_AABB;
	
private:
	int m_ID;
#ifdef GAME
	int m_maximumHealth;
	int m_health;
	eEntityType m_type;
#endif // GAME
#ifdef LEVEL_EDITOR
	eModelName m_modelName;
#endif // LEVEL_EDITOR
	bool m_selected;
};