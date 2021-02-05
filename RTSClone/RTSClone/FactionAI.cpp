#include "FactionAI.h"
#include "AdjacentPositions.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "FactionHandler.h"
#include "GameEvents.h"
#include "Level.h"
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
	const float DELAY_TIMER_EXPIRATION = 25.0f;
	const float IDLE_TIMER_EXPIRATION = 1.0f;
	const float MIN_SPAWN_TIMER_EXPIRATION = 7.5f;
	const float MAX_SPAWN_TIMER_EXPIRATION = 15.0f;
	const int STARTING_WORKER_COUNT = 2;
	const int STARTING_UNIT_COUNT = 1;
	const float MAX_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 18.0f;
	const float MIN_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 3.0f;
	const float DISTANCE_FROM_MINERALS = static_cast<float>(Globals::NODE_SIZE) * 6.0f;

	const int MAX_WORKERS_REPAIR_BUILDING = 2;
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
	int startingResources, int startingPopulationCap, const Base& currentBase)
	: Faction(factionController, hqStartingPosition, startingResources, startingPopulationCap),
	m_spawnQueue(),
	m_actionQueue(),
	m_delayTimer(DELAY_TIMER_EXPIRATION, true),
	m_idleTimer(IDLE_TIMER_EXPIRATION, true),
	m_spawnTimer(Globals::getRandomNumber(MIN_SPAWN_TIMER_EXPIRATION, MAX_SPAWN_TIMER_EXPIRATION), true),
	m_targetFaction(eFactionController::None),
	m_currentBase(currentBase)
{
	for (int i = 0; i < STARTING_WORKER_COUNT; ++i)
	{
		m_spawnQueue.push(eEntityType::Worker);
	}

	m_actionQueue.emplace(eActionType::BuildTurret);
	m_actionQueue.emplace(eActionType::BuildBarracks);
	m_actionQueue.emplace(eActionType::BuildSupplyDepot);
}

void FactionAI::setTargetFaction(FactionHandler& factionHandler)
{
	m_targetFaction = eFactionController::None;

	float targetFactionDistance = std::numeric_limits<float>::max();
	for (const auto& opposingFaction : factionHandler.getOpposingFactions(getController()))
	{
		const Headquarters& opposingHeadquarters = opposingFaction.get().getClosestHeadquarters(m_currentBase.get().position);
		float distance = Globals::getSqrDistance(opposingHeadquarters.getPosition(), m_currentBase.get().position);
		if (distance < targetFactionDistance)
		{
			m_targetFaction = opposingFaction.get().getController();
			targetFactionDistance = distance;
		}
	}
}

void FactionAI::onFactionElimination(FactionHandler& factionHandler, eFactionController eliminatedFaction)
{
	if (m_targetFaction == eliminatedFaction)
	{
		setTargetFaction(factionHandler);
	}
}

void FactionAI::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
	switch (gameEvent.type)
	{
	case eGameEventType::TakeDamage:
	{
		assert(gameEvent.data.takeDamage.senderFaction != getController());
		int targetID = gameEvent.data.takeDamage.targetID;
		auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetID](const auto& entity)
		{
			return entity->getID() == targetID;
		});

		if (entity != m_allEntities.end())
		{
			switch ((*entity)->getEntityType())
			{
			case eEntityType::SupplyDepot:
			break;
			case eEntityType::Barracks:
			break;
			case eEntityType::Headquarters:
				instructWorkersToRepair(static_cast<Headquarters&>(*(*entity)), map);
			break;
			case eEntityType::Turret:
			break;
			}	
		}
	}
	break;
	}

	Faction::handleEvent(gameEvent, map, factionHandler);
}

void FactionAI::selectEntity(const glm::vec3& position)
{
	for (auto& entity : m_allEntities)
	{
		entity->setSelected(entity->getAABB().contains(position));
	}
}

void FactionAI::update(float deltaTime, const Map & map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
	Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

	m_spawnTimer.update(deltaTime);
	if (m_spawnTimer.isExpired())
	{
		m_spawnTimer.resetElaspedTime();
		m_spawnQueue.push(eEntityType::Unit);
	}

	m_delayTimer.update(deltaTime);
	if (m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		if (!m_spawnQueue.empty())
		{
			eEntityType entityTypeToSpawn = m_spawnQueue.front();
			switch (entityTypeToSpawn)
			{
			case eEntityType::Worker:
				assert(!m_headquarters.empty());
				if (m_headquarters.front().addWorkerToSpawnQueue())
				{
					m_spawnQueue.pop();
				}
				break;
			case eEntityType::Unit:
				if (!m_barracks.empty() && m_barracks.front().addUnitToSpawnQueue())
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
			switch (m_actionQueue.front().actionType)
			{
			case eActionType::BuildBarracks:
				onBuild(map, eEntityType::Barracks, factionHandler);
				break;
			case eActionType::BuildSupplyDepot:
				onBuild(map, eEntityType::SupplyDepot, factionHandler);
				break;
			case eActionType::BuildTurret:
				onBuild(map, eEntityType::Turret, factionHandler);
				break;
			default:
				assert(false);
			}
		}
	}

	m_idleTimer.update(deltaTime);
	if (m_idleTimer.isExpired())
	{
		m_idleTimer.resetElaspedTime();

		for (auto& worker : m_workers)
		{
			if (worker.getCurrentState() == eWorkerState::Idle)
			{
				const Mineral& mineralToHarvest = getRandomMineral();
				glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(worker.getPosition(),
					mineralToHarvest.getAABB(), map);

				worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
					eWorkerState::MovingToMinerals, &mineralToHarvest);
			}
		}

		if (m_targetFaction != eFactionController::None)
		{
			const Faction& targetFaction = factionHandler.getFaction(m_targetFaction);
			for (auto& unit : m_units)
			{
				if (unit.getCurrentState() == eUnitState::Idle)
				{
					unit.moveTo(targetFaction.getClosestHeadquarters(unit.getPosition()).getPosition(), map, [&](const glm::ivec2& position)
					{ return getAdjacentPositions(position, map, factionHandler, unit); }, factionHandler, eUnitState::AttackMoving);
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

void FactionAI::instructWorkersToRepair(const Headquarters& HQ, const Map& map)
{
	int repairCount = 0;
	for (auto& worker : m_workers)
	{
		if (worker.getCurrentState() == eWorkerState::Repairing || worker.getCurrentState() == eWorkerState::MovingToRepairPosition)
		{
			++repairCount;

			if (repairCount == MAX_WORKERS_REPAIR_BUILDING)
			{
				break;
			}
		}
		else
		{
			worker.setEntityToRepair(HQ, map);
		}
	}
}

const Mineral& FactionAI::getRandomMineral() const
{
	return m_currentBase.get().minerals[
		Globals::getRandomNumber(0, static_cast<int>(m_currentBase.get().minerals.size()) - 1)];
}

Worker* FactionAI::getAvailableWorker(const glm::vec3& position)
{
	Worker* selectedWorker = nullptr;
	float closestDistance = std::numeric_limits<float>::max();
	for (auto& availableWorker : m_workers)
	{
		float distance = Globals::getSqrDistance(position, availableWorker.getPosition());
		bool selectWorker = false;
		if (!selectedWorker)
		{
			selectWorker = true;
			selectedWorker = &availableWorker;
		}
		else if (availableWorker.getCurrentState() == eWorkerState::Idle &&
			selectedWorker->getCurrentState() != eWorkerState::Idle)
		{
			selectWorker = true;
		}
		else if (availableWorker.getCurrentState() == eWorkerState::Idle &&
			selectedWorker->getCurrentState() == eWorkerState::Idle &&
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

const Entity* FactionAI::spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType)
{
	if (isEntityAffordable(entityType) && !map.isPositionOccupied(position))
	{
		return Faction::spawnBuilding(map, position, entityType);
	}
	
	switch (entityType)
	{
	case eEntityType::SupplyDepot:
		m_actionQueue.push(eActionType::BuildSupplyDepot);
		break;
	case eEntityType::Barracks:
		m_actionQueue.push(eActionType::BuildBarracks);
		break;
	case eEntityType::Turret:
		m_actionQueue.push(eActionType::BuildTurret);
		break;
	default:
		assert(false);
	}
	
	return nullptr;
}

const Entity* FactionAI::spawnUnit(const Map& map, const EntitySpawnerBuilding& building, FactionHandler& factionHandler)
{
	if (isEntityAffordable(eEntityType::Unit) && !isExceedPopulationLimit(eEntityType::Unit))
	{
		return Faction::spawnUnit(map, building, factionHandler);
	}

	m_spawnQueue.push(eEntityType::Unit);
	return nullptr;
}

const Entity* FactionAI::spawnWorker(const Map& map, const EntitySpawnerBuilding& building)
{
	if (isEntityAffordable(eEntityType::Worker) && !isExceedPopulationLimit(eEntityType::Worker))
	{
		return Faction::spawnWorker(map, building);
	}

	m_spawnQueue.push(eEntityType::Worker);
	return nullptr;
}

void FactionAI::onBuild(const Map& map, eEntityType entityTypeToBuild, FactionHandler& factionHandler)
{
	switch (entityTypeToBuild)
	{
	case eEntityType::Barracks:
	case eEntityType::SupplyDepot:
		if (!m_workers.empty())
		{
			glm::vec3 buildPosition(0.0f);
			if (isEntityAffordable(entityTypeToBuild) &&
				PathFinding::getInstance().isBuildingSpawnAvailable(m_headquarters.front().getPosition(),
					ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME), map, buildPosition,
					MIN_DISTANCE_FROM_HQ, MAX_DISTANCE_FROM_HQ, DISTANCE_FROM_MINERALS, *this))
			{
				Worker* availableWorker = getAvailableWorker(buildPosition);
				assert(availableWorker);
				if (availableWorker && instructWorkerToBuild(entityTypeToBuild, buildPosition, map, *availableWorker))
				{
					m_actionQueue.pop();
				}
			}
		}
		break;
	case eEntityType::Turret:
		if (!m_workers.empty() && isEntityAffordable(entityTypeToBuild))
		{
			glm::vec3 buildPosition(0.0f);
			if(isEntityAffordable(entityTypeToBuild) &&
				PathFinding::getInstance().isBuildingSpawnAvailable(m_headquarters.front().getPosition(), 
				ModelManager::getInstance().getModel(TURRET_MODEL_NAME), map, buildPosition, 
				MIN_DISTANCE_FROM_HQ, MAX_DISTANCE_FROM_HQ, DISTANCE_FROM_MINERALS, *this))
			{
				const Faction& opposingFaction = factionHandler.getRandomOpposingFaction(getController());
				if (Globals::getSqrDistance(opposingFaction.getMainHeadquartersPosition(), buildPosition) <=
					Globals::getSqrDistance(opposingFaction.getMainHeadquartersPosition(), m_headquarters.front().getPosition()))
				{
					Worker* availableWorker = getAvailableWorker(buildPosition);
					assert(availableWorker);
					if (availableWorker && instructWorkerToBuild(entityTypeToBuild, buildPosition, map, *availableWorker))
					{
						m_actionQueue.pop();
					}
				}
			}
		}
		break;
	default:
		assert(false);
	}
}