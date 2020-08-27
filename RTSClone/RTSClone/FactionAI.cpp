#include "FactionAI.h"

//Levels
//Strategyt level - general - thgought about game state as a whole  where units are - lacing resources? Or attack enemy base - all high level
//Tactics level - Act on tactics from strategy layer - gain resources or attack enemy - 
//Team Level - made up of units - 

//IF paleyur attacks mid tactic - abondon plan - change tactic
//Each of tactic - in script
//Set of steps: move team 1 to front of enemy base - move team 2 back of enemy base

//Threat detection map - base A* off of the threat of the map - move across area of least threat

//Was tactic successful? If so do it more - Feedback on performance
//Keeping track of wher enemy atacks your base, building turrets in appriopriate places
//Overview of  how AI will work generally.

namespace
{
	constexpr int STARTING_WORKER_COUNT = 5;
	constexpr float DELAY_TIME = 3.0f;
}

FactionAI::FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition)
	: Faction(factionName, hqStartingPosition, mineralsStartingPosition),
	m_unitSpawnQueue(),
	m_delayTimer(DELAY_TIME, true)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_unitSpawnQueue.push(eEntityType::Worker);
	}
}

void FactionAI::update(float deltaTime, const Map & map, const Faction& opposingFaction)
{
	Faction::update(deltaTime, map, opposingFaction);

	m_delayTimer.update(deltaTime);

	if (!m_unitSpawnQueue.empty() && m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		switch (m_unitSpawnQueue.front())
		{
		case eEntityType::Unit:
			if (!m_barracks.empty() && spawnUnit<Unit>(map, m_units, m_unitSpawnQueue.front(), m_barracks.back()))
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
				const Mineral* mineralToHarvest = &m_minerals[mineralIndex];

				glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(addedWorker->getPosition(),
					mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map);
				addedWorker->moveTo(destination, map, eUnitState::MovingToMinerals, mineralToHarvest);
			
				m_unitSpawnQueue.pop();
			}
		}
			break;
		default:
			assert(false);
		}
	}	
}