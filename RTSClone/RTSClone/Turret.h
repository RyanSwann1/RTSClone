#pragma once

#include "Entity.h"
#include "TargetEntity.h"
#include "Timer.h"
#include <optional>

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
	
	void update(float deltaTime, const FactionHandler& factionHandler, const Map& map);

private:
	std::reference_wrapper<const Faction> m_owningFaction;
	std::optional<TargetEntity> m_target	= {};
	Timer m_stateHandlerTimer				= {};
	Timer m_attackTimer						= {};
};