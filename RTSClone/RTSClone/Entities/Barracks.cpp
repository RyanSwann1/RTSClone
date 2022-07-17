#include "Barracks.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Factions/Faction.h"
#include "Core/Map.h"
#include "Core/Level.h"

namespace
{
	constexpr float TIME_BETWEEN_UNIT_SPAWN = 3.0f;
	constexpr float BARRACKS_PROGRESS_BAR_WIDTH = 100.0f;
	constexpr float BARRACKS_PROGRESS_BAR_YOFFSET = 80.0f;
	constexpr int MAX_UNIT_SPAWN_COUNT = 5;
	constexpr EntitySpawnerDetails SPAWN_DETAILS =
	{
		TIME_BETWEEN_UNIT_SPAWN,
		BARRACKS_PROGRESS_BAR_WIDTH,
		BARRACKS_PROGRESS_BAR_YOFFSET,
		MAX_UNIT_SPAWN_COUNT,
		Globals::UNIT_RESOURCE_COST,
		Globals::UNIT_POPULATION_COST
	};
}

Barracks::Barracks(const Position& position, Faction& owningFaction)
	: EntitySpawnerBuilding(position, eEntityType::Barracks, 
		Globals::BARRACKS_STARTING_HEALTH, owningFaction.getCurrentShieldAmount(), SPAWN_DETAILS)
{}

const Entity* Barracks::CreateEntity(Faction & owning_faction, const Map & map)
{
	glm::vec3 position(0.0f);
	if (PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(*this, map, position))
	{
		glm::vec3 rotation = { 0.0f, Globals::getAngle(position, getPosition()), 0.0f };
		return owning_faction.createUnit({ position, rotation, get_waypoint(), eEntityType::Unit, getPosition(), getID() }, map);
	}

	return nullptr;
}
