#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "AABB.h"
#include <functional>
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
	Entity(const Model& model);
	Entity(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3());
	glm::vec3& getRotation();
	glm::vec3& getPosition();
	const Model& getModel() const;
	void setPosition(const glm::vec3& position);
	void setRotation(const glm::vec3 rotation);
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
	const glm::vec3& getRotation() const;
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
	Entity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
		int health = 0, glm::vec3 startingRotation = glm::vec3());
#endif // GAME
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	AABB m_AABB;
	
private:
	int m_ID;
#ifdef GAME
	int m_maximumHealth;
	int m_health;
	eEntityType m_type;
#endif // GAME
	std::reference_wrapper<const Model> m_model;
	bool m_selected;
};