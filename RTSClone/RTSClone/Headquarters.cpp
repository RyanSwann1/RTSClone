#include "Headquarters.h"
#include "ModelManager.h"
#include "Globals.h"
#include "Faction.h"
#include "Level.h"

namespace
{
	const float TIME_BETWEEN_WORKER_SPAWN = 2.0f;
	const int MAX_WORKERS_IN_SPAWN_QUEUE = 5;
	const float HQ_PROGRESS_BAR_WIDTH = 150.0f;
	const float HQ_PROGRESS_BAR_YOFFSET = 220.0f;
}

Headquarters::Headquarters(const glm::vec3& startingPosition, Faction& owningFaction)
	: EntitySpawnerBuilding(startingPosition, eEntityType::Headquarters, TIME_BETWEEN_WORKER_SPAWN,
		Globals::HQ_STARTING_HEALTH, owningFaction, ModelManager::getInstance().getModel(HQ_MODEL_NAME),
		MAX_WORKERS_IN_SPAWN_QUEUE)
{
	Level::add_event(GameEvent::create<AttachFactionToBaseEvent>({ owningFaction.getController(), m_position }));
}

Headquarters::~Headquarters()
{
	Level::add_event(GameEvent::create<DetachFactionFromBaseEvent>({ m_owningFaction.get().getController(), m_position }));
}

void Headquarters::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
	EntitySpawnerBuilding::update(deltaTime, Globals::WORKER_RESOURCE_COST, Globals::WORKER_POPULATION_COST,
		MAX_WORKERS_IN_SPAWN_QUEUE, map, factionHandler);
}

bool Headquarters::addWorkerToSpawnQueue()
{
	if (isEntityAddableToSpawnQueue(MAX_WORKERS_IN_SPAWN_QUEUE, Globals::WORKER_RESOURCE_COST, Globals::WORKER_POPULATION_COST))
	{
		addEntityToSpawnQueue(eEntityType::Worker);
		return true;
	}

	return false;
}

void Headquarters::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_spawnTimer.isActive())
	{
		float currentTime = m_spawnTimer.getElaspedTime() / m_spawnTimer.getExpiredTime();
		float width = HQ_PROGRESS_BAR_WIDTH;
		float yOffset = HQ_PROGRESS_BAR_YOFFSET;

		m_statbarSprite.render(m_position, windowSize, width, width * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

const Entity* Headquarters::spawnEntity(const Map& map, FactionHandler& factionHandler) const
{
	return m_owningFaction.get().createWorker(map, *this);
}