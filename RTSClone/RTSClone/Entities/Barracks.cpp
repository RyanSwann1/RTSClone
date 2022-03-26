#include "Barracks.h"
#include "ModelManager.h"
#include "Globals.h"
#include "Factions/Faction.h"

namespace
{
	const float TIME_BETWEEN_UNIT_SPAWN = 3.0f;
	const int MAX_UNITS_IN_SPAWN_QUEUE = 5;
	const float BARRACKS_PROGRESS_BAR_WIDTH = 100.0f;
	const float BARRACKS_PROGRESS_BAR_YOFFSET = 80.0f;
}

Barracks::Barracks(const glm::vec3& startingPosition, Faction& owningFaction)
	: EntitySpawnerBuilding(startingPosition, eEntityType::Barracks, TIME_BETWEEN_UNIT_SPAWN,
		Globals::BARRACKS_STARTING_HEALTH, owningFaction, ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME), 
		MAX_UNITS_IN_SPAWN_QUEUE)
{}

void Barracks::update(float deltaTime, const Map& map, const FactionHandler& factionHandler)
{
	EntitySpawnerBuilding::update(deltaTime, Globals::UNIT_RESOURCE_COST, Globals::UNIT_POPULATION_COST,
		MAX_UNITS_IN_SPAWN_QUEUE, map, factionHandler);
}

bool Barracks::addUnitToSpawnQueue()
{
	if (isEntityAddableToSpawnQueue(MAX_UNITS_IN_SPAWN_QUEUE, Globals::UNIT_RESOURCE_COST, Globals::UNIT_POPULATION_COST))
	{
		addEntityToSpawnQueue(eEntityType::Worker);
		return true;
	}

	return false;
}

void Barracks::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_spawnTimer.isActive())
	{
		float currentTime = m_spawnTimer.getElaspedTime() / m_spawnTimer.getExpiredTime();
		float width = BARRACKS_PROGRESS_BAR_WIDTH;
		float yOffset = BARRACKS_PROGRESS_BAR_YOFFSET;

		m_statbarSprite.render(m_position, windowSize, width, width * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

const Entity* Barracks::spawnEntity(const Map& map, const FactionHandler& factionHandler) const
{
	return m_owningFaction.get().createUnit(map, *this, factionHandler);
}