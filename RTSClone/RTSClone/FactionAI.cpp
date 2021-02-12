#include "FactionAI.h"
#include "AdjacentPositions.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "FactionHandler.h"
#include "Level.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameEventHandler.h"
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
	const float DELAY_TIMER_EXPIRATION =  10.f;
	const float IDLE_TIMER_EXPIRATION = 1.0f;
	const float MIN_SPAWN_TIMER_EXPIRATION = 7.5f;
	const float MAX_SPAWN_TIMER_EXPIRATION = 15.0f;
	const int STARTING_WORKER_COUNT = 5;
	const int STARTING_UNIT_COUNT = 1;
	const float MAX_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 18.0f;
	const float MIN_DISTANCE_FROM_HQ = static_cast<float>(Globals::NODE_SIZE) * 5.0f;
	const float DISTANCE_FROM_MINERALS = static_cast<float>(Globals::NODE_SIZE) * 7.0f;

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
	int startingResources, int startingPopulationCap, const BaseHandler& baseHandler)
	: Faction(factionController, hqStartingPosition, startingResources, startingPopulationCap),
	m_baseHandler(baseHandler),
	m_spawnQueue(),
	m_actionQueue(),
	m_delayTimer(DELAY_TIMER_EXPIRATION, true),
	m_spawnTimer(Globals::getRandomNumber(MIN_SPAWN_TIMER_EXPIRATION, MAX_SPAWN_TIMER_EXPIRATION), true),
	m_targetFaction(eFactionController::None)
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

	assert(!m_headquarters.empty());
	float targetFactionDistance = std::numeric_limits<float>::max();
	for (const auto& opposingFaction : factionHandler.getOpposingFactions(getController()))
	{
		const Headquarters& opposingHeadquarters = opposingFaction.get().getClosestHeadquarters(m_headquarters.front().getPosition());
		float distance = Globals::getSqrDistance(opposingHeadquarters.getPosition(), m_headquarters.front().getPosition());
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
	Faction::handleEvent(gameEvent, map, factionHandler);

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

		if (entity != m_allEntities.end() &&
			(*entity)->getEntityType() == eEntityType::Headquarters)
		{
			instructWorkersToRepair(static_cast<Headquarters&>(*(*entity)), map);
		}
	}
	break;
	case eGameEventType::OnEnteredIdleState:
	{
		int entityID = gameEvent.data.onEnteredIdleState.entityID;
		switch (gameEvent.data.onEnteredIdleState.entityType)
		{
		case eEntityType::Unit:
		{
			auto unit = std::find_if(m_units.begin(), m_units.end(), [entityID](const auto& unit)
			{
				return unit.getID() == entityID;
			});
			if (unit != m_units.end() && 
				unit->getCurrentState() == eUnitState::Idle &&
				m_targetFaction != eFactionController::None)
			{
				glm::vec3 destination = factionHandler.getFaction(m_targetFaction).getClosestHeadquarters(unit->getPosition()).getPosition();
				unit->moveTo(destination, map, factionHandler, eUnitState::AttackMoving);	
			}
		}
		break;
		case eEntityType::Worker:
		{
			auto worker = std::find_if(m_workers.begin(), m_workers.end(), [entityID](const auto& worker)
			{
				return worker.getID() == entityID;
			});
			if (worker == m_workers.end() || worker->getCurrentState() != eWorkerState::Idle)
			{
				break;
			}
			bool actionCompleted = false;
			if (!m_actionQueue.empty())
			{
				switch (m_actionQueue.front().actionType)
				{
				case eActionType::BuildBarracks:
					if (build(map, eEntityType::Barracks, *worker))
					{
						m_actionQueue.pop();
						actionCompleted = true;
					}
					break;
				case eActionType::BuildSupplyDepot:
					if (build(map, eEntityType::SupplyDepot, *worker))
					{
						m_actionQueue.pop();
						actionCompleted = true;
					}
					break;
				case eActionType::BuildTurret:
					if (build(map, eEntityType::Turret, *worker))
					{
						m_actionQueue.pop();
						actionCompleted = true;
					}
					break;
				default:
					assert(false);
				}
			}
			if (!actionCompleted)
			{
				const Base& nearestBase = m_baseHandler.getNearestBase(getClosestHeadquarters(worker->getPosition()).getPosition());
				const Mineral* nearestMineral = m_baseHandler.getNearestAvailableMineralAtBase(*this, nearestBase, worker->getPosition());
				assert(nearestMineral);
				if (nearestMineral)
				{
					worker->moveTo(*nearestMineral, map);
				}
			}
		}
		break;
		default:
			assert(false);
		}
	}
		break;
	}
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
			if (!m_actionQueue.empty())
			{
				switch (m_actionQueue.front().actionType)
				{
				case eActionType::BuildBarracks:
					if (build(map, eEntityType::Barracks))
					{
						m_actionQueue.pop();
					}
					break;
				case eActionType::BuildSupplyDepot:
					if (build(map, eEntityType::SupplyDepot))
					{
						m_actionQueue.pop();
					}
					break;
				case eActionType::BuildTurret:
					if (build(map, eEntityType::Turret))
					{
						m_actionQueue.pop();
					}
					break;
				default:
					assert(false);
				}
			}

			switch (m_spawnQueue.front())
			{
			case eEntityType::Unit:
			case eEntityType::Worker:
				if (build(map, m_spawnQueue.front()))
				{
					m_spawnQueue.pop();
				}
				break;
			default:
				assert(false);
			}
		}
	}
}

bool FactionAI::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker)
{
	if (map.isWithinBounds(position) && !map.isPositionOccupied(position))
	{
		return worker.build(position, map, entityType);
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
			worker.repairEntity(HQ, map);
		}
	}
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

bool FactionAI::isWithinDistanceOfBuildings(const glm::vec3& position, float distance) const
{
	for (const auto& entity : m_allEntities)
	{
		switch (entity->getEntityType())
		{
		case eEntityType::Headquarters:
		case eEntityType::SupplyDepot:
		case eEntityType::Barracks:
		case eEntityType::Turret:
		case eEntityType::Laboratory:
			if (Globals::getSqrDistance(entity->getPosition(), position) <= distance * distance)
			{
				return true;
			}
			break;
		case eEntityType::Unit:
			break;
		case eEntityType::Worker:
			for (const auto& buildingCommand : static_cast<Worker&>((*entity)).getBuildingCommands())
			{
				if (Globals::getSqrDistance(buildingCommand.position, position) <= distance * distance)
				{
					return true;
				}
			}
			break;
		default:
			assert(false);
		}
	}

	return false;
}

const Entity* FactionAI::createBuilding(const Map& map, const Worker& worker)
{
	const Entity* spawnedBuilding = Faction::createBuilding(map, worker);
	if (!spawnedBuilding)
	{
		switch (worker.getBuildingCommands().front().entityType)
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
	}

	return spawnedBuilding;
}

const Entity* FactionAI::createUnit(const Map& map, const EntitySpawnerBuilding& building, FactionHandler& factionHandler)
{
	const Entity* spawnedUnit = Faction::createUnit(map, building, factionHandler);
	if (!spawnedUnit)
	{
		m_spawnQueue.push(eEntityType::Unit);
	}

	return spawnedUnit;
}

Entity* FactionAI::createWorker(const Map& map, const EntitySpawnerBuilding& building)
{
	Entity* spawnedWorker = Faction::createWorker(map, building);
	if (!spawnedWorker)
	{
		m_spawnQueue.push(eEntityType::Worker);
	}

	return spawnedWorker;
}

bool FactionAI::build(const Map& map, eEntityType entityType)
{
	assert(!m_headquarters.empty());
	if (!isAffordable(entityType))
	{
		return false;
	}

	switch (entityType)
	{
	case eEntityType::Barracks:
	case eEntityType::SupplyDepot:
	case eEntityType::Turret:
	{
		glm::vec3 buildPosition(0.0f);
		if (PathFinding::getInstance().isBuildingSpawnAvailable(m_headquarters.front().getPosition(),
			entityType, map, buildPosition, *this, m_baseHandler))
		{
			Worker* availableWorker = getAvailableWorker(buildPosition);
			if (availableWorker)
			{
				return instructWorkerToBuild(entityType, buildPosition, map, *availableWorker);
			}
		}
	}
		break;
	case eEntityType::Unit:
		return !m_barracks.empty() && m_barracks.front().addUnitToSpawnQueue();
	case eEntityType::Worker:
		return m_headquarters.front().addWorkerToSpawnQueue();
	default:
		assert(false);
	}

	return false;
}

bool FactionAI::build(const Map& map, eEntityType entityType, Worker& worker)
{
	assert(!m_headquarters.empty());
	if (!isAffordable(entityType))
	{
		return false;
	}

	switch (entityType)
	{
	case eEntityType::Barracks:
	case eEntityType::SupplyDepot:
	case eEntityType::Turret:
	{
		glm::vec3 buildPosition(0.0f);
		if (PathFinding::getInstance().isBuildingSpawnAvailable(m_headquarters.front().getPosition(),
			entityType, map, buildPosition, *this, m_baseHandler))
		{
			return instructWorkerToBuild(entityType, buildPosition, map, worker);
		}
	}
	break;
	default:
		assert(false);
	}

	return false;
}