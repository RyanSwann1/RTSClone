#pragma once

#include "Entity.h"
#include "AdjacentPositions.h"
#include "Timer.h"
#include "TargetEntity.h"
#include "Globals.h"
#include "ModelManager.h"
#include "TypeComparison.h"
#include <functional>
#include <vector>
#include <list>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

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
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation);
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation,
		const glm::vec3& destination, FactionHandler& FactionHandler, const Map& map);

	const Faction& getOwningFaction() const;
	const std::vector<glm::vec3>& getPathToPosition() const;
	float getGridAttackRange() const;
	float getAttackRange() const;
	eFactionController getOwningFactionController() const;
	eUnitState getCurrentState() const;

	void resetTarget();
	void moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map, 
		FactionHandler& factionHandler);
	void moveTo(const glm::vec3& destination, const Map& map, FactionHandler& factionHandler, 
		eUnitState state = eUnitState::Moving);
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map,
		const Timer& unitStateHandlerTimer);
	void reduceHealth(const TakeDamageEvent& gameEvent, FactionHandler& factionHandler, const Map& map);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	const Faction& m_owningFaction;
	std::vector<glm::vec3> m_pathToPosition;
	eUnitState m_currentState;
	Timer m_attackTimer;
	TargetEntity m_targetEntity;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING

	void switchToState(eUnitState newState, const Map& map, const Entity* targetEntity = nullptr, const Faction* targetFaction = nullptr);
};