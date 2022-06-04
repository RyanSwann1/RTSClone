#include "Headquarters.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Factions/Faction.h"
#include "Core/Level.h"

namespace
{
	constexpr float TIME_BETWEEN_WORKER_SPAWN = 2.0f;
	constexpr int MAX_WORKERS_IN_SPAWN_QUEUE = 5;
	constexpr float HQ_PROGRESS_BAR_WIDTH = 150.0f;
	constexpr float HQ_PROGRESS_BAR_YOFFSET = 220.0f;
	constexpr int MAX_WORKER_SPAWN_COUNT = 5;
	constexpr EntitySpawnerDetails SPAWN_DETAILS =
	{
		TIME_BETWEEN_WORKER_SPAWN,
		HQ_PROGRESS_BAR_WIDTH,
		HQ_PROGRESS_BAR_YOFFSET,
		MAX_WORKER_SPAWN_COUNT,
		Globals::WORKER_RESOURCE_COST,
		Globals::WORKER_POPULATION_COST
	};
}

Headquarters::Headquarters(const Position& position, Faction& owningFaction)
	: EntitySpawnerBuilding(position, eEntityType::Headquarters, Globals::HQ_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), SPAWN_DETAILS,
		[](Faction& owningFaction, const Map& map, const EntitySpawnerBuilding& spawner) { return owningFaction.createWorker(map, spawner); }),
	m_owningFaction(&owningFaction)
{
	Level::add_event(GameEvent::create<AttachFactionToBaseEvent>({ owningFaction.getController(), m_position.Get() }));
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

Headquarters::~Headquarters()
{
	if (getID() != INVALID_ENTITY_ID)
	{
		Level::add_event(GameEvent::create<DetachFactionFromBaseEvent>({ m_owningFaction->getController(), m_position.Get() }));
		Level::add_event(GameEvent::create<HeadquartersDestroyedEvent>({}));
	}
}