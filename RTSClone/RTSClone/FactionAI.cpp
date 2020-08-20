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
		switch (m_unitSpawnQueue.front())
		{
		case eEntityType::Unit:
			assert(m_barracks.size() == 1);
			if (spawnUnit<Unit>(map, m_units, m_unitSpawnQueue.front(), m_barracks.back()))
			{
				m_unitSpawnQueue.pop();
			}
			break;
		case eEntityType::Worker:
		{
			Worker* addedWorker = spawnUnit<Worker>(map, m_workers, m_unitSpawnQueue.front(), m_HQ);
			if (addedWorker)
			{
				int mineralIndex = Globals::getRandomNumber(0, m_minerals.size() - 1);
				glm::vec3 destination = m_minerals[mineralIndex].getPosition();
				addedWorker->moveTo(destination, map, m_minerals);
			
				m_unitSpawnQueue.pop();
			}
		}
			break;
		default:
			assert(false);
		}
	}	
}