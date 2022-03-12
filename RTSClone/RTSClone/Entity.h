#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include <functional>
#include "EntityType.h"
#include "Sprite.h"
#include "Timer.h"
#include "ActiveStatus.h"

struct TakeDamageEvent;
class FactionHandler;
struct Camera;
class Faction;
enum class eFactionController;
struct Model;
class ShaderHandler;
class Map;
class Entity
{
public:
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;
	Entity(Entity&&) noexcept = default;
	Entity& operator=(Entity&&) noexcept = default;
	virtual ~Entity() {};

	eEntityType getEntityType() const;
	int getMaximumHealth() const;
	int getHealth() const;
	int getShield() const;
	bool isDead() const;
	
	void takeDamage(const TakeDamageEvent& gameEvent, const Map& map, FactionHandler& factionHandler);
	void repair();
	void increaseMaximumShield(const Faction& owningFaction);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void setPosition(const glm::vec3& position);

	int getID() const;
	const glm::vec3& getRotation() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;
	bool isSelected() const;
	
	bool setSelected(bool selected);

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:	
	Entity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
		int health, int shield, glm::vec3 startingRotation = glm::vec3(0.0f));
	
	
	void update(float deltaTime);
	
	ActiveStatus m_status;
	Sprite m_statbarSprite;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	AABB m_AABB;

private:
	int m_ID;
	int m_maximumHealth;
	int m_health;
	int m_maximumShield;
	int m_shield;
	eEntityType m_type;
	Timer m_shieldReplenishTimer;
	std::reference_wrapper<const Model> m_model;
	bool m_selected;

	void increaseShield();
};