#include "FactionAI.h"
#include "AdjacentPositions.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "FactionHandler.h"
#include "Level.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameEventHandler.h"
#include "AIConstants.h"
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
	bool isBaseClosest(const BaseHandler& baseHandler, const glm::vec3& position, const Headquarters& headquarters)
	{
		const Base& baseOnHeadquarters = baseHandler.getBase(headquarters.getPosition());
		for (const auto& base : baseHandler.getBases())
		{
			if (&baseOnHeadquarters != &base &&
				Globals::getSqrDistance(base.getCenteredPosition(), position) < 
				Globals::getSqrDistance(baseOnHeadquarters.getCenteredPosition(), position))
			{
				return false;
			}
		}

		return true;
	}
}

//AIUnattachedToBaseWorkers
AIUnattachedToBaseWorkers::AIUnattachedToBaseWorkers()
	: m_unattachedToBaseWorkers() {}

bool AIUnattachedToBaseWorkers::isEmpty() const
{
	return m_unattachedToBaseWorkers.empty();
}

Worker& AIUnattachedToBaseWorkers::getClosestWorker(const glm::vec3& position)
{
	assert(!m_unattachedToBaseWorkers.empty());
	std::sort(m_unattachedToBaseWorkers.begin(), m_unattachedToBaseWorkers.end(), [&position](const auto& a, const auto& b)
	{
		return Globals::getSqrDistance(position, a.get().getPosition()) > Globals::getSqrDistance(position, b.get().getPosition());
	});
	Worker& worker = m_unattachedToBaseWorkers.back();
	m_unattachedToBaseWorkers.pop_back();
	return worker;
}

void AIUnattachedToBaseWorkers::addWorker(Worker& _worker)
{
	if (std::find_if(m_unattachedToBaseWorkers.cbegin(), m_unattachedToBaseWorkers.cend(), [&_worker](const auto& worker)
	{
		return _worker.getID() == worker.get().getID();
	}) == m_unattachedToBaseWorkers.cend())
	{
		m_unattachedToBaseWorkers.emplace_back(_worker);
	}
}

void AIUnattachedToBaseWorkers::remove(const Worker& _worker)
{
	auto iter = std::find_if(m_unattachedToBaseWorkers.begin(), m_unattachedToBaseWorkers.end(), [&_worker](const auto& worker)
	{
		return _worker.getID() == worker.get().getID();
	});
	assert(iter != m_unattachedToBaseWorkers.end());
	m_unattachedToBaseWorkers.erase(iter);
}

//FactionAI
FactionAI::FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition,
	int startingResources, int startingPopulationCap, const BaseHandler& baseHandler)
	: Faction(factionController, hqStartingPosition, startingResources, startingPopulationCap),
	m_baseHandler(baseHandler),
	m_actionPriorityQueue(AIPriorityActionCompare),
	m_occupiedBases(baseHandler, *this),
	m_baseExpansionTimer(Globals::getRandomNumber(AIConstants::MIN_BASE_EXPANSION_TIME, AIConstants::MAX_BASE_EXPANSION_TIME), true),
	m_currentBehaviour(static_cast<eAIBehaviour>(Globals::getRandomNumber(0, static_cast<int>(eAIBehaviour::Max)))),
	m_actionQueue(),
	m_delayTimer(AIConstants::DELAY_TIMER_EXPIRATION, true),
	m_spawnTimer(Globals::getRandomNumber(AIConstants::MIN_SPAWN_TIMER_EXPIRATION, AIConstants::MAX_SPAWN_TIMER_EXPIRATION), true),
	m_targetFaction(eFactionController::None)
{
	for (const auto& actionType : AIConstants::STARTING_BUILD_ORDERS[Globals::getRandomNumber(0, static_cast<int>(AIConstants::STARTING_BUILD_ORDERS.size() - 1))])
	{
		m_actionQueue.emplace(actionType, *m_occupiedBases.getBase(getMainHeadquartersPosition()));
	}
}

void FactionAI::setTargetFaction(FactionHandler& factionHandler)
{
	m_targetFaction = eFactionController::None;

	assert(!m_headquarters.empty());
	float targetFactionDistance = std::numeric_limits<float>::max();
	for (const auto& opposingFaction : factionHandler.getOpposingFactions(getController()))
	{
		const Headquarters& opposingHeadquarters = opposingFaction.get().getClosestHeadquarters(m_headquarters.front()->getPosition());
		float distance = Globals::getSqrDistance(opposingHeadquarters.getPosition(), m_headquarters.front()->getPosition());
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
			return entity.get().getID() == targetID;
		});

		if (entity != m_allEntities.end() &&
			(*entity).get().getEntityType() == eEntityType::Headquarters)
		{
			instructWorkersToRepair(static_cast<Headquarters&>((*entity).get()), map);
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
				return unit->getID() == entityID;
			});
			if (unit != m_units.end() && 
				(*unit)->getCurrentState() == eUnitState::Idle &&
				m_targetFaction != eFactionController::None)
			{
				if (factionHandler.isFactionActive(m_targetFaction))
				{
					const Faction& targetFaction = factionHandler.getFaction(m_targetFaction);
					const Headquarters& targetHeadquarters = targetFaction.getClosestHeadquarters((*unit)->getPosition());
					(*unit)->moveToAttackPosition(targetHeadquarters, targetFaction, map, factionHandler);
				}
				else
				{
					m_targetFaction = eFactionController::None;
				}
			}
		}
		break;
		case eEntityType::Worker:
		{
			auto worker = std::find_if(m_workers.begin(), m_workers.end(), [entityID](const auto& worker)
			{
				return worker->getID() == entityID;
			});
			if (worker == m_workers.end() || (*worker)->getCurrentState() != eWorkerState::Idle)
			{
				break;
			}
			if (!m_actionQueue.empty() &&
				Globals::BUILDING_TYPES.isMatch(convertActionTypeToEntityType(m_actionQueue.front().actionType)) &&
					build(map, convertActionTypeToEntityType(m_actionQueue.front().actionType), &*(*worker)))
			{
				m_actionQueue.pop();
			}
			else
			{
				const Base& nearestBase = m_baseHandler.getNearestBase(getClosestHeadquarters((*worker)->getPosition()).getPosition());
				const Mineral* nearestMineral = m_baseHandler.getNearestAvailableMineralAtBase(*this, nearestBase, (*worker)->getPosition());
				if (nearestMineral)
				{
					(*worker)->moveTo(*nearestMineral, map);
				}
				else
				{
					for (const auto& base : m_occupiedBases.getSortedBases((*worker)->getPosition()))
					{
						if (&base.base.get() != &nearestBase &&
							base.base.get().owningFactionController == getController())
						{
							nearestMineral = m_baseHandler.getNearestAvailableMineralAtBase(*this, base.base, (*worker)->getPosition());
							if (nearestMineral)
							{
								m_occupiedBases.removeWorker(*(*worker));
								m_occupiedBases.addWorker(*(*worker), base.base);
								(*worker)->moveTo(*nearestMineral, map);
							}
						}
					}
				}
			}
		}
		break;
		default:
			assert(false);
		}
	}
	break;
	case eGameEventType::AttachFactionToBase:
	{
		const Base& base = m_baseHandler.getBase(gameEvent.data.attachFactionToBase.position);
		assert(base.owningFactionController == getController());
		if (!m_unattachedToBaseWorkers.isEmpty())
		{
			for (const auto& mineral : base.minerals)
			{
				Worker& worker = m_unattachedToBaseWorkers.getClosestWorker(base.getCenteredPosition());
				worker.moveTo(mineral, map);
				m_occupiedBases.addWorker(worker, base);
				
				if (m_unattachedToBaseWorkers.isEmpty())
				{
					break;
				}
			}
		}
	}
	break;
	case eGameEventType::DetachFactionFromBase:
	{
		const Base& base = m_baseHandler.getBase(gameEvent.data.detachFactionFromBase.position);
		assert(base.owningFactionController == getController());
		for (auto& worker : m_occupiedBases.getBase(base).workers)
		{
			m_unattachedToBaseWorkers.addWorker(worker);
		}
	}
	break;
	}
}

void FactionAI::selectEntity(const glm::vec3& position)
{
	for (auto& entity : m_allEntities)
	{
		entity.get().setSelected(entity.get().getAABB().contains(position));
	}
}

void FactionAI::update(float deltaTime, const Map & map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
	Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

	m_spawnTimer.update(deltaTime);
	if (m_spawnTimer.isExpired())
	{
		m_spawnTimer.resetElaspedTime();
		//Spawn Unit
	}

	m_delayTimer.update(deltaTime);
	if (m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		if (!m_actionQueue.empty())
		{
			if (handleAction(m_actionQueue.front(), map))
			{
				m_actionQueue.pop();
			}
		}
		else if (!m_actionPriorityQueue.empty())
		{
			if (handleAction(m_actionPriorityQueue.top().action, map))
			{
				m_actionPriorityQueue.pop();
			}
		}
	}

	switch (m_currentBehaviour)
	{
	case eAIBehaviour::Defensive:
		break;
	case eAIBehaviour::Expansive:
		break;
	case eAIBehaviour::Aggressive:
		break;
	default:
		assert(false);
	}

	for (const auto& occupiedBase : m_occupiedBases.getBases())
	{
		if (occupiedBase.base.get().owningFactionController == getController())
		{
			if (occupiedBase.turretCount < AIConstants::MAX_TURRETS_DEFENSIVE)
			{

			}
			if (occupiedBase.supplyDepotCount < AIConstants::MAX_SUPPLY_DEPOT_DEFENSIVE)
			{

			}
			if (occupiedBase.barracksCount < AIConstants::MAX_BARRACKS_DEFENSIVE)
			{

			}
		}
	}

	m_baseExpansionTimer.update(deltaTime);
	if (m_baseExpansionTimer.isExpired() && isAffordable(eEntityType::Headquarters))
	{
		const Base* availableBase = m_baseHandler.getNearestUnusedBase(getMainHeadquartersPosition());
		if (availableBase)
		{
			Worker* availableWorker = getAvailableWorker(availableBase->position);
			if (availableWorker)
			{
				m_baseExpansionTimer.setExpirationTime(200.0f);
				
				AIOccupiedBase* currentBase = m_occupiedBases.getBase(*availableWorker);
				assert(currentBase && currentBase->base.get().owningFactionController == getController());
				m_occupiedBases.removeWorker(*availableWorker);
				
				for (const auto& building : availableWorker->getBuildingCommands())
				{
					m_actionQueue.emplace(convertEntityToActionType(building.entityType), *currentBase);
				}
				availableWorker->build(availableBase->getCenteredPosition(), map, eEntityType::Headquarters, availableBase, true);
				m_unattachedToBaseWorkers.addWorker(*availableWorker);
			}
		}
	}
}

void FactionAI::onEntityRemoval(const Entity& entity)
{
	if (entity.getEntityType() == eEntityType::Worker)
	{
		m_occupiedBases.removeWorker(static_cast<const Worker&>(entity));
	}
	else if(Globals::BUILDING_TYPES.isMatch(entity.getEntityType()))
	{
		m_occupiedBases.removeBuilding(entity);
	}
}

void FactionAI::instructWorkersToRepair(const Headquarters& HQ, const Map& map)
{
	AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(HQ);
	assert(occupiedBase && occupiedBase->base.get().owningFactionController == getController());
	for (auto& worker : occupiedBase->workers)
	{
		if (!worker.get().isRepairing())
		{
			worker.get().repairEntity(HQ, map);
		}
	}
}

Worker* FactionAI::getAvailableWorker(const glm::vec3& position)
{
	if (!m_unattachedToBaseWorkers.isEmpty())
	{
		return &m_unattachedToBaseWorkers.getClosestWorker(position);
	}
	else
	{
		Worker* selectedWorker = nullptr;
		float closestDistance = std::numeric_limits<float>::max();
		for (auto& availableWorker : m_workers)
		{
			float distance = Globals::getSqrDistance(position, availableWorker->getPosition());
			bool selectWorker = false;
			if (!selectedWorker)
			{
				selectWorker = true;
				selectedWorker = &(*availableWorker);
			}
			else if (availableWorker->getCurrentState() == eWorkerState::Idle &&
				selectedWorker->getCurrentState() != eWorkerState::Idle)
			{
				selectWorker = true;
			}
			else if (availableWorker->getCurrentState() == eWorkerState::Idle &&
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
				selectedWorker = &(*availableWorker);
				closestDistance = distance;
			}
		}

		return selectedWorker;
	}
}

Worker* FactionAI::getAvailableWorker(const glm::vec3& position, AIOccupiedBase& occupiedBase)
{
	Worker* selectedWorker = nullptr;
	float closestDistance = std::numeric_limits<float>::max();
	for (auto& availableWorker : occupiedBase.workers)
	{
		float distance = Globals::getSqrDistance(position, availableWorker.get().getPosition());
		bool selectWorker = false;
		if (!selectedWorker)
		{
			selectWorker = true;
			selectedWorker = &availableWorker.get();
		}
		else if (availableWorker.get().getCurrentState() == eWorkerState::Idle &&
			selectedWorker->getCurrentState() != eWorkerState::Idle)
		{
			selectWorker = true;
		}
		else if (availableWorker.get().getCurrentState() == eWorkerState::Idle &&
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
			selectedWorker = &availableWorker.get();
			closestDistance = distance;
		}
	}

	return selectedWorker;
}

bool FactionAI::isWithinRangeOfBuildings(const glm::vec3& position, float distance) const
{
	for (const auto& entity : m_allEntities)
	{
		switch (entity.get().getEntityType())
		{
		case eEntityType::Headquarters:
		case eEntityType::SupplyDepot:
		case eEntityType::Barracks:
		case eEntityType::Turret:
		case eEntityType::Laboratory:
			if (Globals::getSqrDistance(entity.get().getPosition(), position) <= distance * distance)
			{
				return true;
			}
			break;
		case eEntityType::Unit:
			break;
		case eEntityType::Worker:
			for (const auto& buildingCommand : static_cast<Worker&>(entity.get()).getBuildingCommands())
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

void FactionAI::onUnitTakenDamage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler) const
{
	assert(!unit.isDead());
	if (!factionHandler.isFactionActive(gameEvent.senderFaction))
	{
		return;
	}
	
	bool changeTargetEntity = false;
	if(unit.getTargetEntity().getID() != Globals::INVALID_ENTITY_ID)
	{
		if (factionHandler.isFactionActive(unit.getTargetEntity().getFactionController()))
		{
			const Faction& opposingFaction = factionHandler.getFaction(unit.getTargetEntity().getFactionController());
			const Entity* targetEntity = opposingFaction.getEntity(unit.getTargetEntity().getID(), unit.getTargetEntity().getType());
			if (!targetEntity)
			{
				changeTargetEntity = true;
			}
			else if (!Globals::ATTACKING_ENTITY_TYPES.isMatch(targetEntity->getEntityType()) &&
				Globals::ATTACKING_ENTITY_TYPES.isMatch(gameEvent.senderEntityType))
			{
				changeTargetEntity = true;
			}
		}
	}
	else
	{
		changeTargetEntity = true;
	}

	if (changeTargetEntity)
	{
		const Faction& opposingFaction = factionHandler.getFaction(gameEvent.senderFaction);
		const Entity* targetEntity = opposingFaction.getEntity(gameEvent.senderID, gameEvent.senderEntityType);
		if (targetEntity)
		{
			unit.moveToAttackPosition(*targetEntity, opposingFaction, map, factionHandler);
		}	
	}
}

bool FactionAI::increaseShield(const Laboratory& laboratory)
{
	if (!Faction::increaseShield(laboratory))
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(laboratory);
		if (occupiedBase && occupiedBase->base.get().owningFactionController == getController())
		{
			m_actionQueue.emplace(eAIActionType::IncreaseShield, *occupiedBase);
		}
		
		return false;
	}

	return true;
}

const Entity* FactionAI::createBuilding(const Map& map, const Worker& worker)
{
	const Entity* spawnedBuilding = Faction::createBuilding(map, worker);
	if (spawnedBuilding)
	{
		switch (spawnedBuilding->getEntityType())
		{
			case eEntityType::Barracks:
			case eEntityType::SupplyDepot:
			case eEntityType::Turret:
				m_occupiedBases.addBuilding(worker, *spawnedBuilding);
			break;
			case eEntityType::Laboratory:
				m_occupiedBases.addBuilding(worker, *spawnedBuilding);
				if (getCurrentShieldAmount() == 0)
				{
					AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(*spawnedBuilding);
					if (occupiedBase && occupiedBase->base.get().owningFactionController == getController())
					{
						m_actionQueue.emplace(eAIActionType::IncreaseShield, *occupiedBase);
					}
				}
			break;
			case eEntityType::Headquarters:
			break;
			default:
				assert(false);
		}
	}
	else
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(worker);
		if (occupiedBase && occupiedBase->base.get().owningFactionController == getController())
		{
			const glm::vec3& basePosition = occupiedBase->base.get().getCenteredPosition();
			switch (worker.getBuildingCommands().front().entityType)
			{
			case eEntityType::SupplyDepot:
				m_actionQueue.emplace(eAIActionType::BuildSupplyDepot, *occupiedBase);
				break;
			case eEntityType::Barracks:
				m_actionQueue.emplace(eAIActionType::BuildBarracks, *occupiedBase); 
				break;
			case eEntityType::Turret:
				m_actionQueue.emplace(eAIActionType::BuildTurret, *occupiedBase);
				break;
			case eEntityType::Laboratory:
				m_actionQueue.emplace(eAIActionType::BuildLaboratory, *occupiedBase);
				break;
			default:
				assert(false);
			}
		}
	}

	return spawnedBuilding;
}

const Entity* FactionAI::createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler)
{
	const Entity* spawnedUnit = Faction::createUnit(map, barracks, factionHandler);
	if (!spawnedUnit)
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(barracks);
		if (occupiedBase)
		{
			assert(occupiedBase->base.get().owningFactionController == getController());
			m_actionQueue.emplace(eAIActionType::SpawnUnit, *occupiedBase);
		}
	}

	return spawnedUnit;
}

Entity* FactionAI::createWorker(const Map& map, const Headquarters& headquarters)
{
	Entity* spawnedWorker = Faction::createWorker(map, headquarters);
	if (!spawnedWorker)
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(headquarters);
		assert(occupiedBase && occupiedBase->base.get().owningFactionController == getController());
		m_actionQueue.emplace(eAIActionType::SpawnWorker, *occupiedBase);
	}
	else
	{
		assert(spawnedWorker->getEntityType() == eEntityType::Worker);
		m_occupiedBases.addWorker(static_cast<Worker&>(*spawnedWorker), headquarters);
	}

	return spawnedWorker;
}

bool FactionAI::build(const Map& map, eEntityType entityType, Worker* worker, AIOccupiedBase* occupiedBase)
{
	if (!isAffordable(entityType))
	{
		return false;
	}

	switch (entityType)
	{
	case eEntityType::Barracks:
	case eEntityType::SupplyDepot:
	case eEntityType::Turret:
	case eEntityType::Laboratory:
	{
		glm::vec3 buildPosition(0.0f);
		if (PathFinding::getInstance().isBuildingSpawnAvailable(m_headquarters.front()->getPosition(),
			entityType, map, buildPosition, *this, m_baseHandler))
		{
			if (worker)
			{
				return worker->build(buildPosition, map, entityType);
			}
			else
			{
				Worker* availableWorker = occupiedBase ? getAvailableWorker(buildPosition, *occupiedBase) : getAvailableWorker(buildPosition);
				if (availableWorker)
				{
					return availableWorker->build(buildPosition, map, entityType);
				}
			}
		}
	}
		break;
	case eEntityType::Unit:
		assert(!worker);
		return !m_barracks.empty() && m_barracks.front()->addUnitToSpawnQueue();
	case eEntityType::Worker:
		assert(!worker);
		return m_headquarters.front()->addWorkerToSpawnQueue();
	default:
		assert(false);
	}

	return false;
}

bool FactionAI::handleAction(const AIAction& action, const Map& map)
{
	switch (action.actionType)
	{
	case eAIActionType::BuildBarracks:
	case eAIActionType::BuildSupplyDepot:
	case eAIActionType::BuildTurret:
	case eAIActionType::BuildLaboratory:
		if (m_actionQueue.front().base.get().getFactionController() == getController())
		{
			if (build(map, convertActionTypeToEntityType(action.actionType), nullptr, &action.base.get()))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
		break;
	case eAIActionType::IncreaseShield:
		GameEventHandler::getInstance().gameEvents.push(GameEvent::createIncreaseFactionShield(getController()));
		return true;
		break;
	case eAIActionType::SpawnUnit:
	case eAIActionType::SpawnWorker:
		if (build(map, convertActionTypeToEntityType(action.actionType), nullptr))
		{
			return true;
		}
		break;
	default:
		assert(false);
	}

	return false;
}
