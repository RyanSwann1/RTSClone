#pragma once

#include "NonMovable.h"
#include "Entity.h"
#include "TargetEntity.h"
#include "Timer.h"

enum class eTurretState
{
	Idle,
	Attacking
};	

class Faction;
class Map;
class FactionHandler;
class Turret : public Entity, private NonMovable
{
public:
	Turret(const glm::vec3& startingPosition, const Faction& owningFaction);

	void update(float deltaTime, FactionHandler& factionHandler, const Map& map);

private:
	const Faction& m_owningFaction;
	TargetEntity m_targetEntity;
	eTurretState m_currentState;
	Timer m_idleTimer;
	Timer m_attackTimer;
};