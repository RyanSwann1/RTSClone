#include "Faction.h"
#include "GameEventHandler.h"
#include "ModelManager.h"
#include "GameEvents.h"
#include "TypeComparison.h"
#include "FactionHandler.h"

namespace
{
    const size_t MAX_UNITS = 50;
    const size_t MAX_WORKERS = 50;
    const size_t MAX_HEADQUARTERS = 4;
    const size_t MAX_SUPPLY_DEPOTS = 20;
    const size_t MAX_BARRACKS = 20;
    const size_t MAX_TURRETS = 20;
    const size_t MAX_LABORATORIES = 1;

    const size_t MAX_ENTITIES = 
        MAX_UNITS + 
        MAX_WORKERS +
        MAX_HEADQUARTERS +
        MAX_SUPPLY_DEPOTS + 
        MAX_BARRACKS + 
        MAX_TURRETS + 
        MAX_LABORATORIES;
}

Faction::Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    int startingResources, int startingPopulationCap)
    : m_allEntities(),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_barracks(),
    m_turrets(),
    m_headquarters(),
    m_laboratories(),
    m_controller(factionController),
    m_currentResourceAmount(startingResources),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(startingPopulationCap),
    m_currentShieldAmount(0)
{
    m_allEntities.reserve(MAX_ENTITIES);
    m_units.reserve(MAX_UNITS);
    m_workers.reserve(MAX_WORKERS);
    m_headquarters.reserve(MAX_HEADQUARTERS);
    m_supplyDepots.reserve(MAX_SUPPLY_DEPOTS);
    m_barracks.reserve(MAX_BARRACKS);
    m_turrets.reserve(MAX_TURRETS);
    m_laboratories.reserve(MAX_LABORATORIES);

    m_headquarters.emplace_back(std::make_unique<Headquarters>(hqStartingPosition, *this));
    m_allEntities.push_back(*m_headquarters.back());
}

int Faction::getCurrentShieldAmount() const
{
    return m_currentShieldAmount;
}

int Faction::getCurrentPopulationAmount() const
{
    return m_currentPopulationAmount;
}

int Faction::getMaximumPopulationAmount() const
{
    return m_currentPopulationLimit;
}

int Faction::getCurrentResourceAmount() const
{
    return m_currentResourceAmount;
}

const Headquarters& Faction::getMainHeadquarters() const
{
    assert(!m_headquarters.empty());
    return *m_headquarters.front();
}

const Headquarters& Faction::getClosestHeadquarters(const glm::vec3& position) const
{
    const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_headquarters)
    {
        float result = Globals::getSqrDistance(headquarters->getPosition(), position);
        if (result < distance)
        {
            distance = result;
            closestHeadquarters = headquarters.get();
        }
    }

    assert(closestHeadquarters);
    return *closestHeadquarters;
}

const glm::vec3& Faction::getMainHeadquartersPosition() const
{
    assert(!m_headquarters.empty());
    return m_headquarters.front()->getPosition();
}

eFactionController Faction::getController() const
{
    return m_controller;
}

const std::vector<std::unique_ptr<Unit>>& Faction::getUnits() const
{
    return m_units;
}

const std::vector<std::reference_wrapper<Entity>>& Faction::getAllEntities() const
{
    return m_allEntities;
}

const Entity* Faction::getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits) const
{
    const Entity* closestEntity = nullptr;
    float closestEntityDistance = maxDistance * maxDistance;
    
    if (prioritizeUnits)
    {
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity.get().getPosition(), position);
            if (!closestEntity && distance < closestEntityDistance)
            {
                closestEntity = &entity.get();
                closestEntityDistance = distance;
            }
            else if (closestEntity && Globals::BUILDING_TYPES.isMatch(closestEntity->getEntityType()) && 
                Globals::UNIT_TYPES.isMatch(entity.get().getEntityType()) &&
                Globals::getSqrDistance(entity.get().getPosition(), position) < maxDistance * maxDistance)
            {
                closestEntity = &entity.get();
                closestEntityDistance = distance;
            }
            else if (closestEntity && distance < closestEntityDistance)
            {
                closestEntity = &entity.get();
                closestEntityDistance = distance;
            }
        }
    }
    else
    {
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity.get().getPosition(), position);
            if (distance < closestEntityDistance)
            {
                closestEntity = &entity.get();
                closestEntityDistance = distance;
            }
        }
    }

    return closestEntity;
}

const Entity* Faction::getEntity(const AABB& AABB, int entityID, eEntityType entityType) const
{
	const Entity* entity = nullptr;
	switch (entityType)
	{
	case eEntityType::Unit:
		entity = getEntity<std::vector<std::unique_ptr<Unit>>>(m_units, entityID, AABB);
		break;
	case eEntityType::Worker:
		entity = getEntity<std::vector<std::unique_ptr<Worker>>>(m_workers, entityID, AABB);
		break;
	case eEntityType::Headquarters:
		entity = getEntity<std::vector<std::unique_ptr<Headquarters>>>(m_headquarters, entityID, AABB);
		break;
	case eEntityType::SupplyDepot:
		entity = getEntity<std::vector<std::unique_ptr<SupplyDepot>>>(m_supplyDepots, entityID, AABB);
		break;
	case eEntityType::Barracks:
		entity = getEntity<std::vector<std::unique_ptr<Barracks>>>(m_barracks, entityID, AABB);
		break;
	case eEntityType::Turret:
		entity = getEntity<std::vector<std::unique_ptr<Turret>>>(m_turrets, entityID, AABB);
		break;
	case eEntityType::Laboratory:
		entity = getEntity<std::vector<std::unique_ptr<Laboratory>>>(m_laboratories, entityID, AABB);
		break;
	default:
		assert(false);
	}

	return entity;
}

const Entity* Faction::getEntity(const glm::vec3& position) const
{
    auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity.get().getAABB().contains(position);
    });

    if (entity != m_allEntities.cend())
    {
        return &(*entity).get();
    }
    else
    {
        return nullptr;
    }
}

const Entity* Faction::getEntity(int entityID, eEntityType entityType) const
{
    const Entity* entity = nullptr; 
    switch(entityType)
    {
        case eEntityType::Unit:
			entity = getEntity<std::vector<std::unique_ptr<Unit>>>(m_units, entityID);
			break;
        case eEntityType::Worker:
			entity = getEntity<std::vector<std::unique_ptr<Worker>>>(m_workers, entityID);
			break;
        case eEntityType::Headquarters:
			entity = getEntity<std::vector<std::unique_ptr<Headquarters>>>(m_headquarters, entityID);
			break;
        case eEntityType::SupplyDepot:
			entity = getEntity<std::vector<std::unique_ptr<SupplyDepot>>>(m_supplyDepots, entityID);
			break;
        case eEntityType::Barracks:
			entity = getEntity<std::vector<std::unique_ptr<Barracks>>>(m_barracks, entityID);
			break;
        case eEntityType::Turret:
			entity = getEntity<std::vector<std::unique_ptr<Turret>>>(m_turrets, entityID);
			break;
        case eEntityType::Laboratory:
			entity = getEntity<std::vector<std::unique_ptr<Laboratory>>>(m_laboratories, entityID);
			break;
        default:
            assert(false);
    }

    return entity;
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
    switch (gameEvent.type)
    {
    case eGameEventType::TakeDamage:
    {
        assert(gameEvent.data.takeDamage.senderFaction != m_controller);
        int targetID = gameEvent.data.takeDamage.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetID](const auto& entity)
        {
            return entity.get().getID() == targetID;
        });

        if (entity != m_allEntities.end())
        {
            (*entity).get().takeDamage(gameEvent.data.takeDamage, map, factionHandler);
            switch ((*entity).get().getEntityType())
            {
            case eEntityType::Worker:
                if ((*entity).get().isDead())
                {
                    removeEntity<Worker>(m_workers, targetID, entity);
                }
                break;
            case eEntityType::Unit:
                if ((*entity).get().isDead())
                {
                    removeEntity<Unit>(m_units, targetID, entity);
                }
                break;
            case eEntityType::SupplyDepot:
                if ((*entity).get().isDead())
                {
                    removeEntity<SupplyDepot>(m_supplyDepots, targetID, entity);
                }
                break;
            case eEntityType::Barracks:
                if ((*entity).get().isDead())
                {
                    removeEntity<Barracks>(m_barracks, targetID, entity);
                }
                break;
            case eEntityType::Headquarters:
                if ((*entity).get().isDead())
                {
                    removeEntity<Headquarters>(m_headquarters, targetID, entity);
                    if (m_headquarters.empty())
                    {
                        GameEventHandler::getInstance().gameEvents.push(GameEvent::createEliminateFaction(m_controller));
                    }
                }
                break;
            case eEntityType::Turret:
                if ((*entity).get().isDead())
                {
                    removeEntity<Turret>(m_turrets, targetID, entity);
                }
                break;
            case eEntityType::Laboratory:
                if ((*entity).get().isDead())
                {
                    removeEntity<Laboratory>(m_laboratories, targetID, entity);
                }
                break;
            default:
                assert(false);
            }
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        revalidateExistingUnitPaths(map, factionHandler);
        break;
    case eGameEventType::RepairEntity:
    {
        int entityID = gameEvent.data.repairEntity.entityID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [entityID](const auto& entity)
        {
            return entity.get().getID() == entityID;
        });
        if (entity != m_allEntities.end())
        {
            (*entity).get().repair();
        }
    }
        break;
    case eGameEventType::IncreaseFactionShield:
        if (m_currentShieldAmount < Globals::MAX_FACTION_SHIELD_AMOUNT &&
            isAffordable(Globals::FACTION_SHIELD_INCREASE_COST) &&
            m_laboratories.size() == 1)
        {
            m_laboratories.front()->handleEvent(gameEvent.data.increaseFactionShield);
        }
        break;
    case eGameEventType::ForceSelfDestructEntity:
        switch(gameEvent.data.forceSelfDestructEntity.entityType)
        {
		case eEntityType::Worker:
            removeEntity<Worker>(m_workers, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Unit:
            removeEntity<Unit>(m_units, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot>(m_supplyDepots, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Barracks:
            removeEntity<Barracks>(m_barracks, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Headquarters:
            removeEntity<Headquarters>(m_headquarters, gameEvent.data.forceSelfDestructEntity.entityID);
			if (m_headquarters.empty())
			{
				GameEventHandler::getInstance().gameEvents.push(GameEvent::createEliminateFaction(m_controller));
			}
			break;
		case eEntityType::Turret:
            removeEntity<Turret>(m_turrets, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Laboratory:
            removeEntity<Laboratory>(m_laboratories, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		default:
			assert(false);
        }
        break;
    }
}

void Faction::addResources(Worker& worker)
{
    m_currentResourceAmount += worker.extractResources();
}

void Faction::handleWorkerCollisions(const Map& map)
{
    static std::vector<std::reference_wrapper<const Worker>> handledWorkers;

    auto foundWorker = [](int ID) -> bool
    {
        auto worker = std::find_if(handledWorkers.cbegin(), handledWorkers.cend(), [ID](const auto& worker)
        {
            return worker.get().getID() == ID;
        });
        return worker != handledWorkers.cend();
    };

    for (auto& worker : m_workers)
    {
        if (worker->getCurrentState() == eWorkerState::Idle)
        {
            if (map.isCollidable(worker->getPosition()))
            {
                glm::vec3 destination(0.f);
                if(PathFinding::getInstance().getClosestAvailablePosition(*worker, m_workers, map, destination))
                {
                    worker->moveTo(destination, map);
                }
            }
            else
            {
                for (const auto& otherWorker : m_workers)
                {
                    if (worker->getID() != otherWorker->getID() &&
                        foundWorker(otherWorker->getID()) &&
                        otherWorker->getCurrentState() == eWorkerState::Idle &&
                        worker->getAABB().contains(otherWorker->getAABB()))
                    {
                        glm::vec3 destination(0.f);
                        if(PathFinding::getInstance().getClosestAvailablePosition(*worker, m_workers, map, destination))
                        {
							worker->moveTo(destination, map);
							break;
                        }
                    }
                }
            }
        }

        handledWorkers.push_back(*worker.get());
    }

    handledWorkers.clear();
}

void Faction::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
    for (auto& unit : m_units)
    {
        unit->update(deltaTime, factionHandler, map, unitStateHandlerTimer);
    }

    for (auto& worker : m_workers)
    {
        worker->update(deltaTime, map, factionHandler, unitStateHandlerTimer);
    }

    for (auto& barracks : m_barracks)
    {
        barracks->update(deltaTime, map, factionHandler);
    }

    for (auto& turret : m_turrets)
    {
        turret->update(deltaTime, factionHandler, map);
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot->update(deltaTime);
    }

    for (auto& headquarters : m_headquarters)
    {
        headquarters->update(deltaTime, map, factionHandler);
    }

    for (auto& laboratory : m_laboratories)
    {
        laboratory->update(deltaTime);
    }

    if (unitStateHandlerTimer.isExpired())
    {
        handleWorkerCollisions(map);
    }
}

void Faction::render(ShaderHandler& shaderHandler) const
{
    for (const auto& unit : m_units)
    {
        unit->render(shaderHandler, m_controller);
    }

    for (const auto& worker : m_workers)
    {
        worker->render(shaderHandler, m_controller);
    }

    for (const auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot->render(shaderHandler, m_controller);
    }

    for (const auto& barracks : m_barracks)
    {
        barracks->render(shaderHandler, m_controller);
    }

    for (const auto& turret : m_turrets)
    {
        turret->render(shaderHandler, m_controller);
    }

    for (const auto& headquarters : m_headquarters)
    {
        headquarters->render(shaderHandler, m_controller);
    }
 
    for (const auto& laboratory : m_laboratories)
    {
        laboratory->render(shaderHandler, m_controller);
    }
}

void Faction::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
    for (const auto& worker : m_workers)
    {
        worker->renderBuildingCommands(shaderHandler);
    }
}

void Faction::renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
    for (const auto& entity : m_allEntities)
    {
        entity.get().renderHealthBar(shaderHandler, camera, windowSize);
        entity.get().renderShieldBar(shaderHandler, camera, windowSize);

        switch (entity.get().getEntityType())
        {
        case eEntityType::Barracks:
            static_cast<Barracks&>((entity).get()).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Headquarters:
            static_cast<Headquarters&>((entity).get()).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Worker:
            static_cast<Worker&>((entity).get()).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Laboratory:
            static_cast<Laboratory&>((entity).get()).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::SupplyDepot:
        case eEntityType::Turret:
        case eEntityType::Unit:
            break;
        default:
            assert(false);
        }
    }
}

#ifdef RENDER_PATHING
void Faction::renderPathing(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_units)
    {
        unit->renderPathMesh(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker->renderPathMesh(shaderHandler);
    }
}
#endif // RENDER_PATHING

#ifdef RENDER_AABB
void Faction::renderAABB(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_units)
    {
        unit.renderAABB(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.renderAABB(shaderHandler);
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.renderAABB(shaderHandler);
    }

    for (auto& barracks : m_barracks)
    {
        barracks.renderAABB(shaderHandler);
    }

    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

bool Faction::isExceedPopulationLimit(int populationAmount) const
{
    return m_currentPopulationAmount + populationAmount > m_currentPopulationLimit;
}

bool Faction::isExceedPopulationLimit(eEntityType entityType) const
{
    return m_currentPopulationAmount + Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entityType)] > m_currentPopulationLimit;
}

bool Faction::isAffordable(eEntityType entityType) const
{
    return Globals::ENTITY_RESOURCE_COSTS[static_cast<int>(entityType)] <= m_currentResourceAmount;
}

bool Faction::isAffordable(int resourceAmount) const
{
    return resourceAmount <= m_currentResourceAmount;
}

bool Faction::isCollidingWithWorkerBuildQueue(const AABB& AABB) const
{
    for (const auto& worker : m_workers)
    {
        auto buildingCommand = std::find_if(worker->getBuildingCommands().cbegin(), worker->getBuildingCommands().cend(),
            [&AABB](const auto& buildingCommand)
        {
            return AABB.contains(buildingCommand.position);
        });
        if (buildingCommand != worker->getBuildingCommands().cend())
        {
            return true;
        }
    }

    return false;
}

Entity* Faction::createBuilding(const Map& map, const Worker& worker)
{
    assert(worker.getCurrentState() == eWorkerState::Building && !worker.getBuildingCommands().empty());

    eEntityType entityType = worker.getBuildingCommands().front().entityType;
    const glm::vec3& position = worker.getBuildingCommands().front().position;
    if (isAffordable(entityType) && 
        !map.isPositionOccupied(position))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
			if (m_supplyDepots.size() < MAX_SUPPLY_DEPOTS)
			{
				m_supplyDepots.emplace_back(std::make_unique<SupplyDepot>(position, *this));
				addedBuilding = &*m_supplyDepots.back();
				increasePopulationLimit();
			}
            break;
        case eEntityType::Barracks:
            if (m_barracks.size() < MAX_BARRACKS)
            {
				m_barracks.emplace_back(std::make_unique<Barracks>(position, *this));
				addedBuilding = &*m_barracks.back();
            }
            break;
        case eEntityType::Turret:
            if (m_turrets.size() < MAX_TURRETS)
            {
				m_turrets.emplace_back(std::make_unique<Turret>(position, *this));
				addedBuilding = &*m_turrets.back();
            }
            break;
        case eEntityType::Headquarters:
            if (m_headquarters.size() < MAX_HEADQUARTERS)
            {
				m_headquarters.emplace_back(std::make_unique<Headquarters>(position, *this));
				addedBuilding = &*m_headquarters.back();
            }
            break;
        case eEntityType::Laboratory:
            if (m_laboratories.size() < MAX_LABORATORIES)
            {
				m_laboratories.emplace_back(std::make_unique<Laboratory>(position, *this));
				addedBuilding = &*m_laboratories.back();
            }
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            reduceResources(entityType);
            m_allEntities.push_back(*addedBuilding);
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createRevalidateMovementPaths());

            return addedBuilding;
        }
    }

    return nullptr;
}

bool Faction::increaseShield(const Laboratory& laboratory)
{
    assert(laboratory.getShieldUpgradeCounter() > 0);
    if (isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
    {
        ++m_currentShieldAmount;
        m_currentResourceAmount -= Globals::FACTION_SHIELD_INCREASE_COST;

        for (auto& entity : m_allEntities)
        {
            entity.get().increaseMaximumShield(*this);
        }

        return true;
    }
    
    return false;
}

void Faction::reduceResources(eEntityType addedEntityType)
{
    m_currentResourceAmount -= Globals::ENTITY_RESOURCE_COSTS[static_cast<int>(addedEntityType)];
}

void Faction::increaseCurrentPopulationAmount(eEntityType entityType)
{
    m_currentPopulationAmount += Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entityType)];
}

void Faction::decreaseCurrentPopulationAmount(const Entity& entity)
{
    assert(entity.isDead());
    m_currentPopulationAmount -= Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entity.getEntityType())];
}

void Faction::increasePopulationLimit()
{
    m_currentPopulationLimit += Globals::POPULATION_INCREMENT;
}

void Faction::revalidateExistingUnitPaths(const Map& map, FactionHandler& factionHandler)
{
    for (auto& unit : m_units)
    {
        if (!unit->getPathToPosition().empty())
        {
            unit->moveTo(unit->getPathToPosition().front(), map, factionHandler, unit->getCurrentState());
        }
    }

    for (auto& worker : m_workers)
    {
        if (!worker->getPathToPosition().empty())
        {
            switch (worker->getCurrentState())
            {
            case eWorkerState::Moving:
            case eWorkerState::ReturningMineralsToHeadquarters:
            case eWorkerState::MovingToBuildingPosition:
            case eWorkerState::MovingToRepairPosition:
                assert(!worker->getPathToPosition().empty());
                worker->moveTo(worker->getPathToPosition().front(), map, worker->getCurrentState());
                break;
            case eWorkerState::MovingToMinerals:
                assert(worker->getMineralToHarvest());
                worker->moveTo(*worker->getMineralToHarvest(), map);
                break;
            case eWorkerState::Idle:
            case eWorkerState::Harvesting:
            case eWorkerState::Building:
            case eWorkerState::Repairing:
                break;
            default:
                assert(false);
            }
        }
    }
}

bool Faction::isMineralInUse(const Mineral& mineral) const
{
    for (const auto& worker : m_workers)
    {
        if (worker->getMineralToHarvest() && 
            &(*worker->getMineralToHarvest()) == &mineral)
        {
            return true;
        }
    }

    return false;
}

const Entity* Faction::createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler)
{
    assert(barracks.getCurrentSpawnCount() > 0);
    glm::vec3 startingPosition(0.0f);
    if (m_units.size() < MAX_UNITS &&
        isAffordable(eEntityType::Unit) && 
        !isExceedPopulationLimit(eEntityType::Unit) &&
        PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(barracks, map, startingPosition))
    {
        glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, barracks.getPosition()), 0.0f };
        if (barracks.isWaypointActive())
        {
            m_units.emplace_back(std::make_unique<Unit>(*this, startingPosition, startingRotation, barracks.getWaypointPosition(), factionHandler, map));
        }
        else
        {
            m_units.emplace_back(std::make_unique<Unit>(*this, startingPosition, startingRotation, map, factionHandler));
        }

        reduceResources(eEntityType::Unit);
        increaseCurrentPopulationAmount(eEntityType::Unit);
        m_allEntities.push_back(*m_units.back());

        return &*m_units.back();
    }

    return nullptr;
}

Entity* Faction::createWorker(const Map& map, const Headquarters& headquarters)
{
    assert(headquarters.getCurrentSpawnCount() > 0);
    glm::vec3 startingPosition(0.0f);
    if (m_workers.size() < MAX_WORKERS &&
        isAffordable(eEntityType::Worker) && 
        !isExceedPopulationLimit(eEntityType::Worker) &&
        PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(headquarters, map, startingPosition))
    {
        glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, headquarters.getPosition()), 0.0f };
        if (headquarters.isWaypointActive())
        {
            m_workers.emplace_back(std::make_unique<Worker>(*this, startingPosition, headquarters.getWaypointPosition(), map, startingRotation));
        }
        else
        {
            m_workers.emplace_back(std::make_unique<Worker>(*this, map, startingPosition, startingRotation));
        }

        reduceResources(eEntityType::Worker);
        increaseCurrentPopulationAmount(eEntityType::Worker);
        m_allEntities.push_back(*m_workers.back());

        return &*m_workers.back();
    }

    return nullptr;
}