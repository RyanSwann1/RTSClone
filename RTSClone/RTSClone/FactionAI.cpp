#include "FactionAI.h"
#include "AdjacentPositions.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "PathFindingLocator.h"
#include <limits>

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
	constexpr float DELAY_TIME = 23.5f;
	constexpr int STARTING_WORKER_COUNT = 4;
	constexpr int STARTING_UNIT_COUNT = 1;
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
FactionAI::FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
	const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
	int startingPopulation)
	: Faction(factionController, hqStartingPosition, mineralPositions, startingResources, startingPopulation),
	m_spawnQueue(),
	m_actionQueue(),
	m_delayTimer(DELAY_TIME, true),
	m_targetFaction(nullptr)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_spawnQueue.push(eEntityType::Worker);
	}

	m_spawnQueue.push(eEntityType::Unit);

	m_actionQueue.emplace(eActionType::BuildBarracks);
	m_actionQueue.emplace(eActionType::BuildSupplyDepot);
}

void FactionAI::setTargetFaction(const std::vector<const Faction*>& opposingFactions)
{
	m_targetFaction = nullptr;
	float targetFactionDistance = std::numeric_limits<float>::max();
	for (const auto& faction : opposingFactions)
	{
		float distance = Globals::getSqrDistance(faction->getHQPosition(), m_HQ.getPosition());
		if (distance < targetFactionDistance)
		{
			m_targetFaction = faction;
			targetFactionDistance = distance;
		}
	}
}

void FactionAI::update(float deltaTime, const Map & map, FactionHandler& factionHandler)
{
	Faction::update(deltaTime, map, factionHandler);

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
				if (Faction::addUnitToSpawn(entityTypeToSpawn, map, m_HQ))
				{
					m_spawnQueue.pop();
				}
				break;
			case eEntityType::Unit:
				if (!m_barracks.empty() && Faction::addUnitToSpawn(entityTypeToSpawn, map, m_barracks.back()))
				{
					m_spawnQueue.pop();
				}
				break;
			default:
				assert(false);
			}
		}

		if (!m_actionQueue.empty())
		{
			const AIAction& action = m_actionQueue.front();
			switch (action.actionType)
			{
			case eActionType::BuildBarracks:
			{
				glm::vec3 buildPosition = { 0.0f, 0.0f, 0.0f };
				if (isEntityAffordable(eEntityType::Barracks) && 
					PathFindingLocator::get().isBuildingSpawnAvailable(m_HQ.getPosition(), eEntityType::Barracks, map, buildPosition))
				{
					Worker* availableWorker = getAvailableWorker(buildPosition);
					if (availableWorker && instructWorkerToBuild(eEntityType::Barracks, buildPosition, map, *availableWorker))
					{
						m_actionQueue.pop();
					}
				}
			}
				break;
			case eActionType::BuildSupplyDepot:
			{
				glm::vec3 buildPosition = { 0.0f, 0.0f, 0.0f };
				if (isEntityAffordable(eEntityType::SupplyDepot) && 
					PathFindingLocator::get().isBuildingSpawnAvailable(m_HQ.getPosition(), eEntityType::SupplyDepot, map, buildPosition))
				{
					Worker* availableWorker = getAvailableWorker(buildPosition);
					if (availableWorker && instructWorkerToBuild(eEntityType::SupplyDepot, buildPosition, map, *availableWorker))
					{
						m_actionQueue.pop();
					}
				}
			}
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
				glm::vec3 destination = PathFindingLocator::get().getClosestPositionOutsideAABB(worker.getPosition(),
					mineralToHarvest.getAABB(), mineralToHarvest.getPosition(), map);

				worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
					eUnitState::MovingToMinerals, &mineralToHarvest);
			}
		}

		if (m_targetFaction)
		{
			for (auto& unit : m_units)
			{
				if (unit.getCurrentState() == eUnitState::Idle)
				{
					unit.moveTo(m_targetFaction->getHQPosition(), map, [&](const glm::ivec2& position)
					{ return getAdjacentPositions(position, map, m_units, unit); }, eUnitState::AttackMoving);
				}
			}
		}
	}
}

bool FactionAI::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker)
{
	if (map.isWithinBounds(position) && !map.isPositionOccupied(position) && !m_workers.empty())
	{
		return Faction::instructWorkerToBuild(entityType, position, map, worker);
	}

	return false;
}

const Mineral& FactionAI::getRandomMineral() const
{
	assert(!m_minerals.empty());
	return m_minerals[Globals::getRandomNumber(0, static_cast<int>(m_minerals.size()) - 1)];
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