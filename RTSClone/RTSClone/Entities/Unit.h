#pragma once

#include "Entity.h"
#include "Movement.h"
#include "Model/AdjacentPositions.h"
#include "Core/Timer.h"
#include "TargetEntity.h"
#include "Core/Globals.h"
#include "Graphics/ModelManager.h"
#include "Core/TypeComparison.h"
#include <functional>
#include <vector>
#include <queue>
#include <optional>

enum class eUnitState
{
	Idle = 0,
	Moving,
	AttackMoving,
	AttackingTarget,
	Max = AttackingTarget
};

struct EntityToSpawn;
class Faction;
class Map;
class ShaderHandler;
class FactionHandler;
class Unit : public Entity
{
public:
	Unit(Faction& owningFaction, const EntityToSpawn& entity_to_spawn, const Map& map);
	Unit(const Unit&) = delete;
	Unit& operator=(const Unit&) = delete;
	Unit(Unit&&) noexcept = default;
	Unit& operator=(Unit&&) noexcept = default;
	~Unit();

	std::optional<TargetEntity> getTargetEntity() const;
	const std::vector<glm::vec3>& getMovementPath() const;
	float getAttackRange() const;
	eUnitState getCurrentState() const;
	bool is_group_selectable() const override;

	void clear_destinations();
	void attack_entity(const Entity& targetEntity, const eFactionController targetController, const Map& map) override;
	bool MoveTo(const glm::vec3& destination, const Map& map, const bool add_to_destinations) override;
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map);
	void delayed_update(FactionHandler& factionHandler, const Map& map);
	void revalidate_movement_path(const Map& map);
#ifdef RENDER_PATHING
	void render_path(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Movement m_movement						= {};
	eFactionController m_owningFaction		= eFactionController::None;
	eUnitState m_currentState				= eUnitState::Idle;
	Timer m_attackTimer						= {};
	std::optional<TargetEntity> m_target	= {};

	void switchToState(eUnitState newState);
};