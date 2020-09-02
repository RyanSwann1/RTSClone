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
AIAction::AIAction(eEntityType entityTypeToSpawn, eAIImmediateAction immediateAction)
	: entityTypeToSpawn(entityTypeToSpawn),
	immediateAction(immediateAction),
	buildPosition()
{}

AIAction::AIAction(eEntityType entityTypeToSpawn, eAIImmediateAction immediateAction, const glm::vec3& buildPosition)
	: entityTypeToSpawn(entityTypeToSpawn),
	immediateAction(immediateAction),
	buildPosition(buildPosition)
{}

//FactionAI
FactionAI::FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition, 
	const Faction& opposingFaction)
	: Faction(factionName, hqStartingPosition, mineralsStartingPosition),
	m_opposingFaction(opposingFaction),
	m_spawnQueue(),
	m_delayTimer(DELAY_TIME, true)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_spawnQueue.push({ eEntityType::Worker, eAIImmediateAction::Harvest });
	}

	m_spawnQueue.push({ eEntityType::Worker, eAIImmediateAction::BuildSupplyDepot, {35.0f, Globals::GROUND_HEIGHT, 120.0f} });
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
		case eEntityType::Worker:
		{
			Worker* addedWorker = spawnUnit<Worker>(map, m_workers, aiAction.entityTypeToSpawn, m_HQ);
			if (addedWorker)
			{
				switch (aiAction.immediateAction)
				{
				case eAIImmediateAction::Harvest:
				{
					const Mineral& mineralToHarvest = getRandomMineral();
					glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(addedWorker->getPosition(),
						mineralToHarvest.getAABB(), mineralToHarvest.getPosition(), map);

					addedWorker->moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
						eUnitState::MovingToMinerals, &mineralToHarvest);
				}
					break;
				case eAIImmediateAction::BuildSupplyDepot:
					instructWorkerToBuild(eEntityType::SupplyDepot, aiAction.buildPosition, map, *addedWorker);
					break;
				default:
					assert(false);
				}

				m_spawnQueue.pop(); 
			}
		}
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