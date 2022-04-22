#pragma once

#include "Entity.h"
#include "TargetEntity.h"
#include "Core/Timer.h"
#include <optional>

class Faction;
class Map;
class FactionHandler;
class Turret : public Entity
{
public:
	Turret(const glm::vec3& startingPosition, const Faction& owningFaction);
	Turret(Turret&&) noexcept = default;
	Turret& operator=(Turret&&) noexcept = default;
	~Turret();
	
	bool is_singular_selectable_only() const override;

	void update(float deltaTime, const FactionHandler& factionHandler, const Map& map);

private:
	std::reference_wrapper<const Faction> m_owningFaction;
	std::optional<TargetEntity> m_target	= {};
	Timer m_stateHandlerTimer				= {};
	Timer m_attackTimer						= {};
};