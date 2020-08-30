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
	constexpr int STARTING_WORKER_COUNT = 2;
	constexpr float DELAY_TIME = 3.0f;
}

//AIAction
AIAction::AIAction(eEntityType entityTypeToSpawn, const glm::vec3& spawnPosition)
	: entityTypeToSpawn(entityTypeToSpawn),
	spawnPosition(spawnPosition)
{}

//FactionAI
FactionAI::FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition)
	: Faction(factionName, hqStartingPosition, mineralsStartingPosition),
	m_spawnQueue(),
	m_delayTimer(DELAY_TIME, true)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_spawnQueue.push({ eEntityType::Worker, {} });
	}

	m_spawnQueue.push({ eEntityType::SupplyDepot, {35.0f, Globals::GROUND_HEIGHT, 120.0f} });
}

void FactionAI::update(float deltaTime, const Map & map, const Faction& opposingFaction)
{
	Faction::update(deltaTime, map, opposingFaction);

	m_delayTimer.update(deltaTime);

	if (!m_spawnQueue.empty() && m_delayTimer.isExpired())
	{
		const AIAction& aiAction = m_spawnQueue.front();

		switch (aiAction.entityTypeToSpawn)
		{
		case eEntityType::Unit:
			if (!m_barracks.empty() && spawnUnit<Unit>(map, m_units, aiAction.entityTypeToSpawn, m_barracks.back()))
			{
				m_spawnQueue.pop();
			}
			break;
		case eEntityType::Worker:
		{
			Worker* addedWorker = spawnUnit<Worker>(map, m_workers, aiAction.entityTypeToSpawn, m_HQ);
			if (addedWorker)
			{
				const Mineral& mineralToHarvest = getRandomMineral();
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(addedWorker->getPosition(),
					mineralToHarvest.getAABB(), mineralToHarvest.getPosition(), map);

				addedWorker->moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
					eUnitState::MovingToMinerals, &mineralToHarvest);
			
				m_spawnQueue.pop();
			}
		}
			break;
		case eEntityType::SupplyDepot:
			if(instructWorkerToBuild(aiAction.entityTypeToSpawn, aiAction.spawnPosition, map));
			m_spawnQueue.pop();
			break;
		default:
			assert(false);
		}
	}

	if (m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		for (auto& worker : m_workers)
		{
			if (worker.getCurrentState() == eUnitState::Idle)
			{
				const Mineral& mineralToHarvest = getRandomMineral();
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(worker.getPosition(),
					mineralToHarvest.getAABB(), mineralToHarvest.getPosition(), map);

				worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
					eUnitState::MovingToMinerals, &mineralToHarvest);
			}
		}
	}
}

bool FactionAI::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map)
{
	if (Globals::isPositionInMapBounds(position) && !map.isPositionOccupied(position) && !m_workers.empty())
	{
		Worker* closestWorker = nullptr;
		float closestDistance = std::numeric_limits<float>::max();

		for (auto& worker : m_workers)
		{
			float distance = Globals::getSqrDistance(position, worker.getPosition());
			if (distance < closestDistance)
			{
				closestWorker = &worker;
				closestDistance = distance;
			}
		}

		assert(closestWorker);
		return Faction::instructWorkerToBuild(entityType, position, map, *closestWorker);
	}

	return false;
}

const Mineral& FactionAI::getRandomMineral() const
{
	assert(!m_minerals.empty());
	return m_minerals[Globals::getRandomNumber(0, m_minerals.size() - 1)];
}