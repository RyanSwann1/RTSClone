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
	: EntitySpawnerBuilding(position, eEntityType::Headquarters, Globals::HQ_STARTING_HEALTH, 
		owningFaction.getCurrentShieldAmount(), SPAWN_DETAILS),
	m_owningFaction(&owningFaction)
{
	Level::add_event(GameEvent::create<AttachFactionToBaseEvent>({ owningFaction.getController(), m_position.Get() }));
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

Headquarters::~Headquarters()
{
	if (getID() != UniqueID::INVALID_ID)
	{
		Level::add_event(GameEvent::create<DetachFactionFromBaseEvent>({ m_owningFaction->getController(), m_position.Get() }));
		Level::add_event(GameEvent::create<HeadquartersDestroyedEvent>({}));
	}
}

const Entity* Headquarters::CreateEntity(Faction& owning_faction, const Map& map)
{
	glm::vec3 position(0.0f);
	if (PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(*this, map, position))
	{
		glm::vec3 rotation = { 0.0f, Globals::getAngle(position, getPosition()), 0.0f };
		return owning_faction.createWorker({ position, rotation, get_waypoint(), eEntityType::Worker, getPosition(), getID() }, map);
	}

	return nullptr;
}