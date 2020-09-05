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
	constexpr float DELAY_TIME = 3.0f;
	constexpr int STARTING_WORKER_COUNT = 4;
}

//AIAction
AIAction::AIAction(eActionType actionType)
	: actionType(actionType),
	position()
{}

AIAction::AIAction(eActionType actionType, const glm::vec3& position)
	: actionType(actionType),
	position(position)
{}

//FactionAI
FactionAI::FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition, 
	const Faction& opposingFaction)
	: Faction(factionName, hqStartingPosition, mineralsStartingPosition),
	m_opposingFaction(opposingFaction), 
	m_spawnQueue(),
	m_actionQueue(),
	m_delayTimer(DELAY_TIME, true)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_spawnQueue.push(eEntityType::Worker);
	}

	m_actionQueue.push({ eActionType::BuildSupplyDepot, {35.0f, Globals::GROUND_HEIGHT, 120.0f} });
	m_actionQueue.push({ eActionType::BuildBarracks, {45.0f, Globals::GROUND_HEIGHT, 120.0f} });
}

void FactionAI::update(float deltaTime, const Map & map, const Faction& opposingFaction)
{
	Faction::update(deltaTime, map, opposingFaction);

	m_delayTimer.update(deltaTime);

	if (m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		if(!m_spawnQueue.empty())
		{
			eEntityType entityTypeToSpawn = m_spawnQueue.front();
			switch (entityTypeToSpawn)
			{
			case eEntityType::Worker:
				m_HQ.addUnitToSpawn([&](const UnitSpawnerBuilding& building) 
					{ return spawnUnit<Worker>(map, m_workers, eEntityType::Worker, building); });
				break;
			case eEntityType::Unit:

				break;
			default:
				assert(false);
			}

			m_spawnQueue.pop();
		}

		if (!m_actionQueue.empty())
		{
			const AIAction& action = m_actionQueue.front();
			switch (action.actionType)
			{
			case eActionType::BuildBarracks:
			{
				if (isEntityAffordable(eEntityType::Barracks))
				{
					Worker* availableWorker = getAvailableWorker(action.position);
					if (availableWorker && instructWorkerToBuild(eEntityType::Barracks, action.position, map, *availableWorker))
					{
						m_actionQueue.pop();
					}
				}
			}
				break;
			case eActionType::BuildSupplyDepot:
				
				break;
			default:
				assert(false);
			}
		}

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

bool FactionAI::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker)
{
	if (Globals::isPositionInMapBounds(position) && !map.isPositionOccupied(position) && !m_workers.empty())
	{
		return Faction::instructWorkerToBuild(entityType, position, map, worker);
	}

	return false;
}

const Mineral& FactionAI::getRandomMineral() const
{
	assert(!m_minerals.empty());
	return m_minerals[Globals::getRandomNumber(0, m_minerals.size() - 1)];
}

Worker* FactionAI::getAvailableWorker(const glm::vec3& position)
{
	if (m_workers.empty())
	{
		return nullptr;
	}
	
	Worker* selectedWorker = nullptr;
	float closestDistance = std::numeric_limits<float>::max();
	for (auto& availableWorker : m_workers)
	{
		float distance = Globals::getSqrDistance(position, availableWorker.getPosition());
		bool selectWorker = false;
		if (!selectedWorker)
		{
			selectWorker = true;
		}
		else if (availableWorker.getCurrentState() == eUnitState::Idle &&
			selectedWorker->getCurrentState() != eUnitState::Idle)
		{
			selectWorker = true;
		}
		else if (availableWorker.getCurrentState() == eUnitState::Idle &&
			selectedWorker->getCurrentState() == eUnitState::Idle &&
			distance < closestDistance)
		{
			selectWorker = true;
		}
		else if (distance < closestDistance)
		{
			selectWorker = true;
		}

		if (selectWorker)
		{
			selectedWorker = &availableWorker;
			closestDistance = distance;
		}
	}

	return selectedWorker;
}