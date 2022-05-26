#pragma once

#include "glm/glm.hpp"
#include "Core/AABB.h"
#include <functional>
#include "EntityType.h"
#include "UI/Sprite.h"
#include "Core/Timer.h"

constexpr int INVALID_ENTITY_ID = -1;

struct EntityID
{
	EntityID();
	EntityID(const EntityID&) = delete;
	EntityID& operator=(const EntityID&) = delete;
	EntityID(EntityID&& rhs) noexcept;
	EntityID& operator=(EntityID&& rhs) noexcept;

	int id = INVALID_ENTITY_ID;
};

struct TakeDamageEvent;
class FactionHandler;
struct Camera;
class Faction;
enum class eFactionController;
struct Model;
class ShaderHandler;
class Map;
class Mineral;
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
	virtual bool is_singular_selectable_only() const = 0;

	void takeDamage(const TakeDamageEvent& gameEvent, const Map& map);
	void repair();
	void increaseMaximumShield(const Faction& owningFaction);
	virtual bool repairEntity(const Entity& entity, const Map& map);
	virtual void harvest(const Mineral& mineral, const Map& map) {}
	virtual void set_waypoint_position(const glm::vec3& position, const Map& map) {};
	virtual void attack_entity(const Entity& targetEntity, const eFactionController targetController, const Map& map) {};
	virtual void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	virtual void render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

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
	void setPosition(const glm::vec3& position);
	
	Sprite m_statbarSprite;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	AABB m_AABB;

private:
	EntityID m_id					= {};
	int m_maximumHealth				= 0;
	int m_health					= 0;
	int m_maximumShield				= 0;
	int m_shield					= 0;
	eEntityType m_type				= {};
	Timer m_shieldReplenishTimer	= {};
	std::reference_wrapper<const Model> m_model;
	bool m_selected					= false;

	void increaseShield();
	void renderHealthBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void renderShieldBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
};