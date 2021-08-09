#pragma once

#include "MovingEntity.h"
#include "AdjacentPositions.h"
#include "Timer.h"
#include "TargetEntity.h"
#include "Globals.h"
#include "ModelManager.h"
#include "TypeComparison.h"
#include <functional>
#include <vector>

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
class Unit : public MovingEntity
{
public:
	Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map,
		FactionHandler& factionHandler);
	Unit(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation,
		const glm::vec3& destination, FactionHandler& factionHandler, const Map& map);
	~Unit();

	TargetEntity getTargetEntity() const;
	const Faction& getOwningFaction() const;
	const std::vector<glm::vec3>& getMovementPath() const;
	float getAttackRange() const;
	eFactionController getOwningFactionController() const;
	eUnitState getCurrentState() const;

	void moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map, 
		FactionHandler& factionHandler);
	void moveTo(const glm::vec3& destination, const Map& map, FactionHandler& factionHandler,
		eUnitState state = eUnitState::Moving);
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map,
		const Timer& unitStateHandlerTimer);
	void takeDamage(const TakeDamageEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	std::reference_wrapper<Faction> m_owningFaction;
	eUnitState m_currentState;
	Timer m_attackTimer;
	TargetEntity m_targetEntity;

	void switchToState(eUnitState newState, const Map& map, FactionHandler& factionHandler,
		const Entity* targetEntity = nullptr, const Faction* targetFaction = nullptr);
};