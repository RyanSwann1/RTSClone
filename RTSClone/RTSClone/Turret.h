#pragma once

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
class Turret : public Entity
{
public:
	Turret(const glm::vec3& startingPosition, const Faction& owningFaction);
	Turret(Turret&&) = default;
	Turret& operator=(Turret&&) = default;
	~Turret();
	
	void update(float deltaTime, FactionHandler& factionHandler, const Map& map);

private:
	std::reference_wrapper<const Faction> m_owningFaction;
	TargetEntity m_targetEntity;
	eTurretState m_currentState;
	Timer m_stateHandlerTimer;
	Timer m_attackTimer;

	void switchToState(eTurretState newState);
};