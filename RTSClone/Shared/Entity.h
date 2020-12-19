#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"
#include "AABB.h"
#include <functional>
#ifdef GAME
#include "EntityType.h"
#include "Sprite.h"
struct TakeDamageEvent;
class FactionHandler;
struct Camera;
class Faction;
#endif // GAME
enum class eFactionController;
struct Model;
class ShaderHandler;
class Map;
class Entity : private NonCopyable
{
public:
	virtual ~Entity();
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
	bool isDead() const;
	
	void reduceHealth(const TakeDamageEvent& gameEvent);
	void repair();
	void increaseShield(const Faction& owningFaction);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
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
	int m_maximumShield;
	int m_shield;
	int m_maximumHealth;
	int m_health;
	eEntityType m_type;
	Sprite m_healthbarSprite;
#endif // GAME
	std::reference_wrapper<const Model> m_model;
	bool m_selected;
};