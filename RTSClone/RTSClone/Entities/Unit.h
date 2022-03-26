#pragma once

#include "Entity.h"
#include "Movement.h"
#include "AdjacentPositions.h"
#include "Timer.h"
#include "TargetEntity.h"
#include "Globals.h"
#include "ModelManager.h"
#include "TypeComparison.h"
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

class Faction;
class Map;
class ShaderHandler;
class FactionHandler;
class Unit : public Entity
{
public:
	Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map);
	Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation,
		const glm::vec3& destination, const Map& map);
	~Unit();

	std::optional<TargetEntity> getTargetEntity() const;
	const std::vector<glm::vec3>& getMovementPath() const;
	float getAttackRange() const;
	eUnitState getCurrentState() const;

	void add_destination(const glm::vec3& position, const Map& map);
	void clear_destinations();
	void moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map);
	void moveTo(const glm::vec3& destination, const Map& map, eUnitState state = eUnitState::Moving);
	void update(float deltaTime, const FactionHandler& factionHandler, const Map& map);
	void delayed_update(const FactionHandler& factionHandler, const Map& map);
#ifdef RENDER_PATHING
	void render_path(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Movement m_movement							= {};
	eFactionController m_owningFaction			= eFactionController::None;
	eUnitState m_currentState					= eUnitState::Idle;
	Timer m_attackTimer							= {};
	std::optional<TargetEntity> m_target		= {};

	void switchToState(eUnitState newState, const Map& map, const Entity* targetEntity = nullptr, const Faction* targetFaction = nullptr);
};