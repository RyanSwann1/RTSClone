#include "FactionAI.h"

namespace
{
	constexpr int STARTING_WORKER_COUNT = 5;
}

FactionAI::FactionAI(const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition)
	: Faction(hqStartingPosition, mineralsStartingPosition),
	m_unitSpawnQueue()
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_unitSpawnQueue.push(eEntityType::Worker);
	}
}

void FactionAI::update(float deltaTime, const Map & map)
{
	Faction::update(deltaTime, map);

	if (!m_unitSpawnQueue.empty())
	{
		bool unitSpawned = false;
		switch (m_unitSpawnQueue.front())
		{
		case eEntityType::Unit:
			assert(m_barracks.size() == 1);
			unitSpawned = spawnUnit<Unit>(map, m_units, m_unitSpawnQueue.front(), m_barracks.back());
			break;
		case eEntityType::Worker:
			unitSpawned = spawnUnit<Worker>(map, m_workers, m_unitSpawnQueue.front(), m_HQ);
			break;
		default:
			assert(false);
		}

		if (unitSpawned)
		{
			m_unitSpawnQueue.pop();
		}
	}	
}