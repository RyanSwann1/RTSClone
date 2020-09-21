#pragma once

#include "Entity.h"
#include "AdjacentPositions.h"
#include "Timer.h"
#include "EntityTarget.h"
#include "Globals.h"
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
	Building
};

class Faction;
class Map;
class ShaderHandler;
class FactionHandler;
class Unit : public Entity, private NonMovable
{
public:
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, eEntityType entityType = eEntityType::Unit, int health = Globals::UNIT_STARTING_HEALTH);
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& destinationPosition, 
		const Map& map, eEntityType entityType = eEntityType::Unit, int health = Globals::UNIT_STARTING_HEALTH);
		 
	bool isPathEmpty() const;
	const glm::vec3& getDestination() const;
	eUnitState getCurrentState() const;

	void resetTarget();
	void setTarget(eFactionController targetFaction, int targetID);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions, 
		eUnitState state = eUnitState::Moving);
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	const Faction& m_owningFaction;
	eUnitState m_currentState;
	glm::vec3 m_front;
	std::vector<glm::vec3> m_pathToPosition;

	void switchToState(eUnitState newState, const Map& map, const Entity* targetEntity = nullptr);

private:
	Timer m_attackTimer;
	Timer m_stateHandlerTimer;
	EntityTarget m_target;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};