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
	MovingToMinerals,
	ReturningMineralsToHQ,
	Harvesting,
	MovingToBuildingPosition,
	SetAttackPosition,
	AttackingTarget,
	Building,
	MovingToRepairPosition,
	Repairing,
	Max = Repairing
};

class Faction;
class Map;
class ShaderHandler;
class FactionHandler;
class Unit : public Entity, private NonMovable
{
public:
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition);

	const std::vector<glm::vec3>& getPathToPosition() const;
	float getGridAttackRange() const;
	float getAttackRange() const;
	eFactionController getOwningFactionController() const;
	eUnitState getCurrentState() const;

	void resetTarget();
	void moveToAttackPosition(const Entity& targetEntity, const Faction& targetFaction, const Map& map, 
		FactionHandler& factionHandler);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
		FactionHandler& factionHandler, eUnitState state = eUnitState::Moving);
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map,
		const Timer& unitStateHandlerTimer);
	void reduceHealth(const TakeDamageEvent& gameEvent, FactionHandler& factionHandler, const Map& map);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition,
		eEntityType entityType, int health, const Model& model);

	const Faction& m_owningFaction;
	std::vector<glm::vec3> m_pathToPosition;

	void switchToState(eUnitState newState, const Map& map, const Entity* targetEntity = nullptr);

private:
	eUnitState m_currentState;
	Timer m_attackTimer;
	TargetEntity m_targetEntity;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};