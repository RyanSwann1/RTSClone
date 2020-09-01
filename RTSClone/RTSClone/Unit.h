#pragma once

#include "Entity.h"
#include "AdjacentPositions.h"
#include "Timer.h"
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
class Unit : public Entity, private NonMovable
{
public:
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, eEntityType entityType = eEntityType::Unit);
	Unit(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& destinationPosition, 
		const Map& map, eEntityType entityType = eEntityType::Unit);
		 
	int getTargetID() const;
	bool isPathEmpty() const;
	const glm::vec3& getDestination() const;
	eUnitState getCurrentState() const;

	void resetTargetID();
	void setTargetID(int entityTargetID);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const GetAllAdjacentPositions& getAdjacentPositions, 
		eUnitState state = eUnitState::Moving);
	void update(float deltaTime, const Faction& opposingFaction, const Map& map);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	const Faction& m_owningFaction;
	eUnitState m_currentState;
	glm::vec3 m_front;
	std::vector<glm::vec3> m_pathToPosition;

private:
	Timer m_attackTimer;
	int m_targetEntityID;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};