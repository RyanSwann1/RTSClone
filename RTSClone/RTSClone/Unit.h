#pragma once

#include "Entity.h"
#include "AdjacentPositions.h"
#include "Timer.h"
#include "TargetEntity.h"
#include "Globals.h"
#include "ModelManager.h"
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
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, 
		eEntityType entityType = eEntityType::Unit, int health = Globals::UNIT_STARTING_HEALTH,
		const Model& model = ModelManager::getInstance().getModel(UNIT_MODEL_NAME));
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& destinationPosition, 
		const Map& map, eEntityType entityType = eEntityType::Unit, int health = Globals::UNIT_STARTING_HEALTH, 
		const Model& model = ModelManager::getInstance().getModel(UNIT_MODEL_NAME));

	float getGridAttackRange() const;
	float getAttackRange() const;
	bool isPathEmpty() const;
	const glm::vec3& getDestination() const;
	eUnitState getCurrentState() const;

	void resetTarget();
	void setTarget(eFactionController targetFaction, int targetID);
	void moveToAttackPosition(const Entity& targetEntity, eFactionController targetFaction, const Map& map);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
		eUnitState state = eUnitState::Moving);
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	const Faction& m_owningFaction;
	std::vector<glm::vec3> m_pathToPosition;

	void switchToState(eUnitState newState, const Map& map, const Entity* targetEntity = nullptr);

private:
	eUnitState m_currentState;
	Timer m_attackTimer;
	Timer m_stateHandlerTimer;
	TargetEntity m_targetEntity;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};