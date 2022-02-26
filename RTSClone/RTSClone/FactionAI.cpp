#include "FactionAI.h"
#include "AdjacentPositions.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "FactionHandler.h"
#include "Level.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include <limits>
#include <algorithm>

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
		if (const Base* baseOnHeadquarters = baseHandler.getBase(headquarters.getPosition()))
		{
			return std::any_of(baseHandler.getBases().cbegin(), baseHandler.getBases().cend(), [&](const auto& base)
			{
				return &*baseOnHeadquarters != &base &&
					Globals::getSqrDistance(base.getCenteredPosition(), position) <
					Globals::getSqrDistance(baseOnHeadquarters->getCenteredPosition(), position);
			});
		}

		return false;
	}

	void removeFromSquad(std::vector<AISquad>& squads, const Unit& unit)
	{
		for (auto squad = squads.begin(); squad != squads.end(); ++squad)
		{
			auto iter = std::find_if(squad->begin(), squad->end(), [&unit](const auto& squadUnit)
			{
				return squadUnit.get().getID() == unit.getID();
			});
			if (iter != squad->end())
			{
				squad->erase(iter);
				if (squad->empty())
				{
					squads.erase(squad);
				}
				break;
			}
		}
	}

	AISquad* getSquad(std::vector<AISquad>& squads, const Unit& unit)
	{
		for (auto& squad : squads)
		{
			auto iter = std::find_if(squad.begin(), squad.end(), [&unit](const auto& unitInSquad)
			{
				return unitInSquad.get().getID() == unit.getID();
			});
			if (iter != squad.end())
			{
				return &squad;
			}
		}

		return nullptr;
	}

	bool isWorkerSpawnable(const AIOccupiedBase& occupiedBase, int cap)
	{
		return occupiedBase.getQueuedAIActionTypeCount(eAIActionType::SpawnWorker) + 
			static_cast<int>(occupiedBase.workers.size()) < cap; 
	}

	bool isTurretSpawnable(const AIOccupiedBase& occupiedBase, int cap)
	{
		return occupiedBase.getQueuedAIActionTypeCount(eAIActionType::BuildTurret) + 
			occupiedBase.getWorkerBuildQueueCount(eEntityType::Turret) + 
			occupiedBase.turretCount < cap;
	}

	bool isBarracksSpawnable(const AIOccupiedBase& occupiedBase, int cap)
	{
		return occupiedBase.getQueuedAIActionTypeCount(eAIActionType::BuildBarracks) +
			occupiedBase.getWorkerBuildQueueCount(eEntityType::Barracks) +
			occupiedBase.barracksCount < cap;
	}

	bool isSupplyDepotSpawnable(const AIOccupiedBase& occupiedBase, int cap)
	{
		return occupiedBase.getQueuedAIActionTypeCount(eAIActionType::BuildSupplyDepot) +
			occupiedBase.getWorkerBuildQueueCount(eEntityType::SupplyDepot) +
			occupiedBase.supplyDepotCount < cap;
	}

	constexpr int MAX_UNITS_ON_HOLD = 3;
}

//FactionAI
FactionAI::FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition,
	int startingResources, int startingPopulationCap, AIConstants::eBehaviour behaviour, const BaseHandler& baseHandler)
	: Faction(factionController, hqStartingPosition, startingResources, startingPopulationCap),
	m_baseHandler(baseHandler),
	m_behaviour(behaviour),
	m_occupiedBases(baseHandler, getController()),
	m_baseExpansionTimer(Globals::getRandomNumber(AIConstants::MIN_BASE_EXPANSION_TIME, AIConstants::MAX_BASE_EXPANSION_TIME), true),
	m_delayTimer(AIConstants::DELAY_TIMER_EXPIRATION, true),
	m_spawnTimer(Globals::getRandomNumber(AIConstants::MIN_SPAWN_TIMER_EXPIRATION, AIConstants::MAX_SPAWN_TIMER_EXPIRATION), true),
	m_targetFaction(eFactionController::None)
{
	m_unitsOnHold.reserve(m_units.capacity());
}

void FactionAI::setTargetFaction(FactionHandler& factionHandler)
{
	m_targetFaction = eFactionController::None;

	assert(!m_headquarters.empty());
	const auto& opposingFactions = factionHandler.getOpposingFactions(getController());
	std::for_each(opposingFactions.cbegin(), opposingFactions.cend(), 
		[&, targetFactionDistance = std::numeric_limits<float>::max()](const auto& opposingFaction) mutable
	{
		if (const Headquarters* opposingHeadquarters = opposingFaction.get().getClosestHeadquarters(m_headquarters.front()->getPosition()))
		{
			float distance = Globals::getSqrDistance(opposingHeadquarters->getPosition(), m_headquarters.front()->getPosition());
			if (distance < targetFactionDistance)
			{
				m_targetFaction = opposingFaction.get().getController();
				targetFactionDistance = distance;
			}
		}
	});
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
		auto entity = std::find_if(m_entities.begin(), m_entities.end(), [targetID](const auto& entity)
		{
			return entity->getID() == targetID;
		});

		if (entity != m_entities.end() &&
			(*entity)->getEntityType() == eEntityType::Headquarters)
		{
			instructWorkersToRepair(*(*entity), map);
		}
	}
	break;
	case eGameEventType::AttachFactionToBase:
	{	
		if (const Base* base = m_baseHandler.getBase(gameEvent.data.attachFactionToBase.position))
		{
			AIOccupiedBase& occupiedBase = m_occupiedBases.addBase(*base);
			if (m_occupiedBases.bases.size() == 1)
			{
				for (const auto& actionType : AIConstants::STARTING_BUILD_ORDERS[static_cast<size_t>(m_behaviour)])
				{
					occupiedBase.actionQueue.emplace_back(actionType);
				}
			}
			else if (!m_unattachedToBaseWorkers.isEmpty())
			{
				for (const auto& mineral : occupiedBase.base.get().minerals)
				{
					Worker& worker = m_unattachedToBaseWorkers.getClosestWorker(mineral.getPosition());
					worker.moveTo(mineral, map);
					occupiedBase.addWorker(worker);

					if (m_unattachedToBaseWorkers.isEmpty())
					{
						break;
					}
				}
			}
		}
	}
	break;
	case eGameEventType::DetachFactionFromBase:
	{
		if (const Base* base = m_baseHandler.getBase(gameEvent.data.attachFactionToBase.position))
		{
			for (auto& worker : m_occupiedBases.removeBase(*base))
			{
				m_unattachedToBaseWorkers.addWorker(worker);
			}
		}
	}
	break;
	}
}

void FactionAI::selectEntity(const glm::vec3& position)
{
	for (auto& entity : m_entities)
	{
		entity->setSelected(entity->getAABB().contains(position));
	}
}

void FactionAI::update(float deltaTime, const Map & map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
	Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

	m_delayTimer.update(deltaTime);
	if (m_delayTimer.isExpired())
	{
		m_delayTimer.resetElaspedTime();

		std::for_each(m_occupiedBases.bases.begin(), m_occupiedBases.bases.end(), [&map, this](auto& occupiedBase)
		{
			//Update action queues
			if (!occupiedBase.actionQueue.empty())
			{
				if (handleAction(occupiedBase.actionQueue.front(), map, occupiedBase))
				{
					occupiedBase.actionQueue.pop_front();
				}
			}
			if (!occupiedBase.actionPriorityQueue.empty())
			{
				if (handleAction(occupiedBase.actionPriorityQueue.top(), map, occupiedBase))
				{
					occupiedBase.actionPriorityQueue.pop();
				}
			}

			//Worker
			if (isWorkerSpawnable(occupiedBase, AIConstants::MIN_WORKERS_AT_BASE))
			{
				occupiedBase.actionQueue.emplace_back(eAIActionType::SpawnWorker);
			}
			else if (isWorkerSpawnable(occupiedBase, static_cast<int>(occupiedBase.base.get().minerals.size())))
			{
				occupiedBase.actionPriorityQueue.emplace(getEntityModifier(eEntityType::Worker, m_behaviour), eAIActionType::SpawnWorker);
			}

			//Turret
			if (isTurretSpawnable(occupiedBase, AIConstants::getMaxTurretCount(m_behaviour)))
			{
				occupiedBase.actionPriorityQueue.emplace(getEntityModifier(eEntityType::Turret, m_behaviour), eAIActionType::BuildTurret);
			}
			//Barracks
			if (isBarracksSpawnable(occupiedBase, AIConstants::getMaxBarracksCount(m_behaviour)))
			{
				occupiedBase.actionPriorityQueue.emplace(getEntityModifier(eEntityType::Barracks, m_behaviour), eAIActionType::BuildBarracks);
			}
			//Supply Depot
			if (isSupplyDepotSpawnable(occupiedBase, AIConstants::getMaxSupplyDepotCount(m_behaviour)))
			{
				occupiedBase.actionPriorityQueue.emplace(getEntityModifier(eEntityType::SupplyDepot, m_behaviour), eAIActionType::BuildSupplyDepot);
			}
		});
	}

	switch (m_behaviour)
	{
	case AIConstants::eBehaviour::Defensive:
		break;
	case AIConstants::eBehaviour::Aggressive:
		m_spawnTimer.update(deltaTime);
		if (m_spawnTimer.isExpired())
		{
			m_spawnTimer.resetElaspedTime();
			const Headquarters* mainHeadquarters = getMainHeadquarters();
			if (AIOccupiedBase* base;
				mainHeadquarters && (base = m_occupiedBases.getBase(*mainHeadquarters)))
			{
				build(map, eEntityType::Unit, *base);
			}
		}
		break;
	default:
		assert(false);
	}

	m_baseExpansionTimer.update(deltaTime);
	if (const Headquarters* mainHeadquarters;
		m_baseExpansionTimer.isExpired() 
		&& isAffordable(eEntityType::Headquarters)
		&& (mainHeadquarters = getMainHeadquarters()))
	{
		const Base* availableBase = m_baseHandler.getNearestUnusedBase(mainHeadquarters->getPosition());
		if (availableBase)
		{
			Worker* availableWorker = getAvailableWorker(availableBase->position);
			if (availableWorker)
			{
				m_baseExpansionTimer.setExpirationTime(200.0f);
				
				AIOccupiedBase* currentBase = m_occupiedBases.getBase(*availableWorker);
				if (currentBase)
				{
					m_occupiedBases.removeWorker(*availableWorker);

					for (const auto& building : availableWorker->get_scheduled_buildings())
					{
						currentBase->actionQueue.emplace_back(convertEntityToActionType(building.entityType));
					}
				}	

				availableWorker->build(availableBase->getCenteredPosition(), map, eEntityType::Headquarters, true);
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
	else if (entity.getEntityType() == eEntityType::Unit)
	{
		removeFromSquad(m_squads, static_cast<const Unit&>(entity));
	}
	else if(Globals::BUILDING_TYPES.isMatch(entity.getEntityType()))
	{
		m_occupiedBases.removeBuilding(entity);
	}
}

void FactionAI::on_entity_taken_damage(const TakeDamageEvent& gameEvent, Entity& entity, const Map& map, FactionHandler& factionHandler)
{
	assert(!entity.isDead());

	switch (entity.getEntityType())
	{
	case eEntityType::Unit:
		on_unit_taken_damage(gameEvent, static_cast<Unit&>(entity), map, factionHandler);
		break;
	}
}

void FactionAI::on_entity_idle(Entity& entity, const Map& map, FactionHandler& factionHandler)
{
	switch (entity.getEntityType())
	{
	case eEntityType::Unit:
		on_unit_idle(static_cast<Unit&>(entity), map, factionHandler);
		break;
	case eEntityType::Worker:
		on_worker_idle(static_cast<Worker&>(entity), map);
		break;
	}
}

void FactionAI::instructWorkersToRepair(const Entity& entity, const Map& map)
{
	AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(entity);
	assert(occupiedBase && occupiedBase->base.get().owningFactionController == getController());
	for (auto& worker : occupiedBase->workers)
	{
		if (!worker.get().isRepairing())
		{
			worker.get().repairEntity(entity, map);
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
	for (const auto& entity : m_entities)
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
			for (const auto& buildingCommand : static_cast<Worker&>(*entity).get_scheduled_buildings())
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

		return false;
	}
}

bool FactionAI::increaseShield(const Laboratory& laboratory)
{
	if (!Faction::increaseShield(laboratory))
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(laboratory);
		if (occupiedBase)
		{
			occupiedBase->actionQueue.emplace_back(eAIActionType::IncreaseShield);
		}
		
		return false;
	}

	return true;
}

Entity* FactionAI::createBuilding(const Map& map, const Worker& worker)
{
	Entity* spawnedBuilding = Faction::createBuilding(map, worker);
	if (spawnedBuilding)
	{
		switch (spawnedBuilding->getEntityType())
		{
			case eEntityType::SupplyDepot:
			case eEntityType::Turret:
			case eEntityType::Barracks:
				m_occupiedBases.addBuilding(worker, *spawnedBuilding);
			break;
			case eEntityType::Laboratory:
				m_occupiedBases.addBuilding(worker, *spawnedBuilding);
				if (getCurrentShieldAmount() == 0)
				{
					AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(*spawnedBuilding);
					if (occupiedBase)
					{
						occupiedBase->actionQueue.emplace_back(eAIActionType::IncreaseShield);
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
		if (occupiedBase)
		{
			const glm::vec3& basePosition = occupiedBase->base.get().getCenteredPosition();
			switch (worker.get_scheduled_buildings().front().entityType)
			{
			case eEntityType::SupplyDepot:
				occupiedBase->actionQueue.emplace_back(eAIActionType::BuildSupplyDepot);
				break;
			case eEntityType::Barracks:
				occupiedBase->actionQueue.emplace_back(eAIActionType::BuildBarracks); 
				break;
			case eEntityType::Turret:
				occupiedBase->actionQueue.emplace_back(eAIActionType::BuildTurret);
				break;
			case eEntityType::Laboratory:
				occupiedBase->actionQueue.emplace_back(eAIActionType::BuildLaboratory);
				break;
			default:
				assert(false);
			}
		}
	}

	return spawnedBuilding;
}

Entity* FactionAI::createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler)
{
	Entity* spawnedUnit = Faction::createUnit(map, barracks, factionHandler);
	if (!spawnedUnit)
	{
		AIOccupiedBase* occupiedBase = m_occupiedBases.getBase(barracks);
		if (occupiedBase)
		{
			assert(occupiedBase->base.get().owningFactionController == getController());
			occupiedBase->actionQueue.emplace_back(eAIActionType::SpawnUnit);
		}
	}
	else
	{
		assert(std::find_if(m_unitsOnHold.cbegin(), m_unitsOnHold.cend(), [spawnedUnit](const auto& unit)
		{
			return unit->getID() == spawnedUnit->getID();
		}) == m_unitsOnHold.cend());

		assert(spawnedUnit->getEntityType() == eEntityType::Unit);
		m_unitsOnHold.push_back(static_cast<Unit*>(spawnedUnit));
		if (m_unitsOnHold.size() == MAX_UNITS_ON_HOLD)
		{
			m_squads.emplace_back();
			for (auto iter = m_unitsOnHold.begin(); iter != m_unitsOnHold.end();)
			{
				Unit& unitOnHold = *(*iter);
				m_squads.back().push_back(unitOnHold);
				iter = m_unitsOnHold.erase(iter);
				on_unit_idle(unitOnHold, map, factionHandler);
			}
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
		assert(occupiedBase);
		occupiedBase->actionQueue.emplace_back(eAIActionType::SpawnWorker);
	}
	else
	{
		assert(spawnedWorker->getEntityType() == eEntityType::Worker);
		m_occupiedBases.addWorker(static_cast<Worker&>(*spawnedWorker), headquarters);
	}

	return spawnedWorker;
}

bool FactionAI::build(const Map& map, eEntityType entityType, AIOccupiedBase& occupiedBase, Worker* worker)
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
		if (PathFinding::getInstance().isBuildingSpawnAvailable(occupiedBase.base.get().getCenteredPosition(),
			entityType, map, buildPosition, *this, m_baseHandler))
		{
			if (worker)
			{
				return worker->build(buildPosition, map, entityType);
			}
			else
			{
				Worker* availableWorker = getAvailableWorker(buildPosition, occupiedBase);
				if (availableWorker)
				{
					return availableWorker->build(buildPosition, map, entityType);
				}
			}
		}
	}
		break;
	case eEntityType::Unit:
	{
		assert(!worker);
		Entity* barracks = occupiedBase.getBuilding(eEntityType::Barracks);
		if (barracks)
		{
			return static_cast<Barracks&>(*barracks).addUnitToSpawnQueue();
		}
	}
		break;
	case eEntityType::Worker:
	{
		assert(!worker);
		const glm::vec3& basePosition = occupiedBase.base.get().getCenteredPosition();
		auto headquarters = std::find_if(m_headquarters.begin(), m_headquarters.end(), [&basePosition](const auto& headquarters)
		{
			return headquarters->getPosition() == basePosition;
		});
		return headquarters != m_headquarters.end() ? (*headquarters)->addWorkerToSpawnQueue() : false;
	}
		break;
	default:
		assert(false);
	}

	return false;
}

bool FactionAI::handleAction(const AIAction& action, const Map& map, AIOccupiedBase& occupiedBase)
{
	switch (action.actionType)
	{
	case eAIActionType::BuildBarracks:
	case eAIActionType::BuildSupplyDepot:
	case eAIActionType::BuildTurret:
	case eAIActionType::BuildLaboratory:
	{
		eEntityType entityType;
		if (convertActionTypeToEntityType(action.actionType, entityType))
		{
			if (build(map, entityType, occupiedBase, nullptr))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}

		break;
	case eAIActionType::IncreaseShield:
		Level::add_event(GameEvent::createIncreaseFactionShield(getController()));
		return true;
	case eAIActionType::SpawnUnit:
	case eAIActionType::SpawnWorker:
	{
		eEntityType entityType;
		if (convertActionTypeToEntityType(action.actionType, entityType) &&
			build(map, entityType, occupiedBase, nullptr))
		{
			return true;
		}
	}
		break;
	default:
		assert(false);
	}

	return false;
}

void FactionAI::on_unit_taken_damage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler)
{
	assert(!unit.isDead());
	if (!factionHandler.isFactionActive(gameEvent.senderFaction))
	{
		return;
	}

	bool changeTargetEntity = false;
	if (std::optional<TargetEntity> target = unit.getTargetEntity())
	{
		if (const Faction* opposingFaction = factionHandler.getFaction(target->controller))
		{
			const Entity* targetEntity = opposingFaction->getEntity(target->ID, target->type);
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

	if (const Faction* opposingFaction;
		changeTargetEntity
		&& (opposingFaction = factionHandler.getFaction(gameEvent.senderFaction)))
	{
		const Entity* targetEntity = opposingFaction->getEntity(gameEvent.senderID, gameEvent.senderEntityType);
		if (targetEntity)
		{
			AISquad* squad = getSquad(m_squads, unit);
			if (squad)
			{
				for (auto& unitInSquad : *squad)
				{
					unitInSquad.get().moveToAttackPosition(*targetEntity, *opposingFaction, map, factionHandler);
				}
			}
		}
	}
}

void FactionAI::on_unit_idle(Unit& unit, const Map& map, FactionHandler& factionHandler)
{
	assert(unit.getCurrentState() == eUnitState::Idle);
	int unitID = unit.getID();
	auto unitOnHold = std::find_if(m_unitsOnHold.cbegin(), m_unitsOnHold.cend(), [unitID](const auto& unit)
	{
		return unit->getID() == unitID;
	});
	if (unitOnHold == m_unitsOnHold.cend() && m_targetFaction != eFactionController::None)
	{
		if (const Faction* targetFaction = factionHandler.getFaction(m_targetFaction))
		{
			if (const Headquarters* nearestHeadquarters = targetFaction->getClosestHeadquarters(unit.getPosition()))
			{
				unit.moveToAttackPosition(*nearestHeadquarters, *targetFaction, map, factionHandler);
			}
		}
		else
		{
			m_targetFaction = eFactionController::None;
		}
	}
}

void FactionAI::on_worker_idle(Worker& worker, const Map& map)
{
	assert(worker.getCurrentState() == eWorkerState::Idle);
	if (const Headquarters* nearestHeadquarters = getClosestHeadquarters(worker.getPosition()))
	{
		if (const Base* nearestBase = m_baseHandler.getNearestBase(nearestHeadquarters->getPosition()))
		{
			if (const Mineral* nearestMineral = m_baseHandler.getNearestAvailableMineralAtBase(*this, *nearestBase, worker.getPosition()))
			{
				worker.moveTo(*nearestMineral, map);
			}
			else
			{
				for (const auto& base : m_occupiedBases.getSortedBases(worker.getPosition()))
				{
					if (&base.base.get() != &*nearestBase)
					{
						nearestMineral = m_baseHandler.getNearestAvailableMineralAtBase(*this, base.base, worker.getPosition());
						if (nearestMineral)
						{
							m_occupiedBases.removeWorker(worker);
							m_occupiedBases.addWorker(worker, base.base);
							worker.moveTo(*nearestMineral, map);
						}
					}
				}
			}
		}
	}


	if (worker.getCurrentState() == eWorkerState::Idle)
	{
		for (auto& occupiedBase : m_occupiedBases.bases)
		{
			if (!occupiedBase.actionQueue.empty())
			{
				eEntityType entityType;
				if (convertActionTypeToEntityType(occupiedBase.actionQueue.front().actionType, entityType) &&
					Globals::BUILDING_TYPES.isMatch(entityType) &&
					build(map, entityType, occupiedBase, &worker))
				{
					occupiedBase.actionQueue.pop_front();
					break;
				}
			}
		}
	}
}
