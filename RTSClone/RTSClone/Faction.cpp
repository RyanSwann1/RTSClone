#include "Faction.h"
#include "GameEventHandler.h"
#include "ModelManager.h"
#include "GameEvent.h"
#include "TypeComparison.h"
#include "FactionHandler.h"

namespace
{
    std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION> initializeMinerals(
        const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions)
    {
        //Done in this way because of how the constructors/move constructors are setup
        int i = 0;
        std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION> minerals =
        {
            mineralPositions[i],
            mineralPositions[++i],
            mineralPositions[++i],
            mineralPositions[++i],
            mineralPositions[++i]
        };

        assert(i + 1 == static_cast<int>(mineralPositions.size()));

        return minerals;
    };
}

Faction::Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
    int startingPopulationCap)
    : m_plannedBuildings(),
    m_minerals(initializeMinerals(mineralPositions)),
    m_allEntities(),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_barracks(),
    m_HQ(Globals::convertToNodePosition(hqStartingPosition), *this),
    m_controller(factionController),
    m_currentResourceAmount(startingResources),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(startingPopulationCap)
{
    m_allEntities.push_back(&m_HQ);
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

const glm::vec3& Faction::getHQPosition() const
{
    return m_HQ.getPosition();
}

eFactionController Faction::getController() const
{
    return m_controller;
}

const std::list<Unit>& Faction::getUnits() const
{
    return m_units;
}

const Entity* Faction::getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits) const
{
    const Entity* closestEntity = nullptr;
    float closestEntityDistance = maxDistance * maxDistance;
    
    if (prioritizeUnits)
    {
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (!closestEntity && distance < closestEntityDistance)
            {
                closestEntity = entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && Globals::BUILDING_TYPES.isMatch(closestEntity->getEntityType()) && 
                Globals::UNIT_TYPES.isMatch(entity->getEntityType()) &&
                Globals::getSqrDistance(entity->getPosition(), position) < maxDistance * maxDistance)
            {
                closestEntity = entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && distance < closestEntityDistance)
            {
                closestEntity = entity;
                closestEntityDistance = distance;
            }
        }
    }
    else
    {
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (distance < closestEntityDistance)
            {
                closestEntity = entity;
                closestEntityDistance = distance;
            }
        }
    }

    return closestEntity;
}

const Entity* Faction::getEntity(const AABB& AABB, int entityID) const
{
    auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&AABB, entityID](const auto& entity)
    {
        return entity->getAABB().contains(AABB) && entity->getID() == entityID;
    });
    
    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }

    return nullptr;
}

const Entity* Faction::getEntity(int entityID) const
{
    auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [entityID](const auto& entity)
    {
        return entity->getID() == entityID;
    });
    
    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }

    return nullptr;
}

const Entity* Faction::getEntity(const glm::vec3& position) const
{
    auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });

    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }
    else
    {
        return nullptr;
    }
}

const std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION>& Faction::getMinerals() const
{
    return m_minerals;
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map)
{
    switch (gameEvent.type)
    {
    case eGameEventType::Attack:
    {
        assert(gameEvent.senderFaction != m_controller);
        int targetID = gameEvent.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetID](const auto& entity)
        {
            return entity->getID() == targetID;
        });

        if (entity != m_allEntities.end())
        {
            (*entity)->reduceHealth(gameEvent.damage);
            if ((*entity)->isDead())
            {
                switch ((*entity)->getEntityType())
                {
                case eEntityType::Worker:
                {
                    decreaseCurrentPopulationAmount(*(*entity));
                    auto worker = std::find_if(m_workers.begin(), m_workers.end(), [targetID](const auto& worker)
                    {
                        return worker.getID() == targetID;
                    });
                    assert(worker != m_workers.end());

                    m_workers.erase(worker);
                }
                break;
                case eEntityType::SupplyDepot:
                {
                    auto supplyDepot = std::find_if(m_supplyDepots.begin(), m_supplyDepots.end(), [targetID](const auto& supplyDepot)
                    {
                        return supplyDepot.getID() == targetID;
                    });
                    assert(supplyDepot != m_supplyDepots.end());

                    m_supplyDepots.erase(supplyDepot);
                }
                break;
                case eEntityType::Barracks:
                {
                    auto barracks = std::find_if(m_barracks.begin(), m_barracks.end(), [targetID](const auto& barracks)
                    {
                        return barracks.getID() == targetID;
                    });
                    assert(barracks != m_barracks.end());

                    m_barracks.erase(barracks);
                }
                break;
                case eEntityType::HQ:
                    GameEventHandler::getInstance().gameEvents.push({ eGameEventType::EliminateFaction, m_controller });
                    break;
                case eEntityType::Unit:
                {
                    decreaseCurrentPopulationAmount(*(*entity));
                    auto unit = std::find_if(m_units.begin(), m_units.end(), [targetID](const auto& unit)
                    {
                        return unit.getID() == targetID;
                    });
                    assert(unit != m_units.end());

                    m_units.erase(unit);
                }
                break;
                case eEntityType::Turret:
                {
                    auto turret = std::find_if(m_turrets.begin(), m_turrets.end(), [targetID](const auto& turret)
                    {
                        return turret.getID() == targetID;
                    });
                    assert(turret != m_turrets.cend());

                    m_turrets.erase(turret);
                }
                    break;
                default:
                    assert(false);
                }

                m_allEntities.erase(entity);
            }   
        }
    }
        break;
    case eGameEventType::RemovePlannedBuilding:
    {
        assert(gameEvent.senderFaction == m_controller);
        const glm::vec3& buildingPosition = gameEvent.startingPosition;
        auto buildingToSpawn = std::find_if(m_plannedBuildings.begin(), m_plannedBuildings.end(), [&buildingPosition](const auto& buildingToSpawn)
        {
            return buildingToSpawn.getPosition() == buildingPosition;
        });

        assert(buildingToSpawn != m_plannedBuildings.end());
        m_plannedBuildings.erase(buildingToSpawn);
    }
        break;
    case eGameEventType::RemoveAllWorkerPlannedBuildings:
        for (auto plannedBuilding = m_plannedBuildings.begin(); plannedBuilding != m_plannedBuildings.end();)
        {
            if (plannedBuilding->getWorkerID() == gameEvent.senderID)
            {
                plannedBuilding = m_plannedBuildings.erase(plannedBuilding);
            }
            else
            {
                ++plannedBuilding;
            }
        }
        break;
    case eGameEventType::AddResources:
    {
        int workerID = gameEvent.senderID;
        auto worker = std::find_if(m_workers.begin(), m_workers.end(), [workerID](const auto& worker)
        {
            return worker.getID() == workerID;
        });
        if (worker != m_workers.end())
        {
            addResources(*worker);
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        revalidateExistingUnitPaths(map);
        break;
    case eGameEventType::RepairEntity:
    {
        int entityID = gameEvent.senderID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [entityID](const auto& entity)
        {
            return entity->getID() == entityID;
        });
        if (entity != m_allEntities.end())
        {
            (*entity)->repair();
        }
    }
        break;
    }
}

void Faction::addResources(Worker& worker)
{
    m_currentResourceAmount += worker.extractResources();
}

void Faction::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime, factionHandler, map);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, m_HQ, map, factionHandler);
    }

    for (auto& barracks : m_barracks)
    {
        barracks.update(deltaTime);
    }

    for (auto& turret : m_turrets)
    {
        turret.update(deltaTime, factionHandler, map);
    }

    m_HQ.update(deltaTime);

    handleCollisions<Unit>(m_units, map);
    handleCollisions<Worker>(m_workers, map);
}

void Faction::render(ShaderHandler& shaderHandler) const
{
    m_HQ.render(shaderHandler, m_controller);

    for (const auto& unit : m_units)
    {
        unit.render(shaderHandler, m_controller);
    }

    for (const auto& worker : m_workers)
    {
        worker.render(shaderHandler, m_controller);
    }

    for (const auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.render(shaderHandler, m_controller);
    }

    for (const auto& barracks : m_barracks)
    {
        barracks.render(shaderHandler, m_controller);
    }

    for (const auto& minerals : m_minerals)
    {
        minerals.render(shaderHandler, m_controller);
    }

    for (const auto& turret : m_turrets)
    {
        turret.render(shaderHandler, m_controller);
    }
}

void Faction::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
    for (const auto& plannedBuilding : m_plannedBuildings)
    {
        plannedBuilding.render(shaderHandler, m_controller);
    }
}

#ifdef RENDER_PATHING
void Faction::renderPathing(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_units)
    {
        unit.renderPathMesh(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.renderPathMesh(shaderHandler);
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

    for (auto& minerals : m_minerals)
    {
        minerals.renderAABB(shaderHandler);
    }

    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

bool Faction::isExceedPopulationLimit(eEntityType entityType) const
{
    switch (entityType)
    {
    case eEntityType::Unit:
        return m_currentPopulationAmount + Globals::UNIT_POPULATION_COST > m_currentPopulationLimit;
    case eEntityType::Worker:
        return m_currentPopulationAmount + Globals::WORKER_POPULATION_COST > m_currentPopulationLimit;
    default:
        assert(false);
        return true;
    }
}

bool Faction::isEntityAffordable(eEntityType entityType) const
{
    switch (entityType)
    {
    case eEntityType::Worker:
        return m_currentResourceAmount - Globals::WORKER_RESOURCE_COST >= 0;
    case eEntityType::Unit:
        return  m_currentResourceAmount - Globals::UNIT_RESOURCE_COST >= 0;
    case eEntityType::SupplyDepot:
        return m_currentResourceAmount - Globals::SUPPLY_DEPOT_RESOURCE_COST >= 0;
    case eEntityType::Barracks:
        return m_currentResourceAmount - Globals::BARRACKS_RESOURCE_COST >= 0;
    case eEntityType::Turret:
        return m_currentResourceAmount - Globals::TURRET_RESOURCE_COST >= 0;
    default:
        assert(false);
        return false;
    }
}

const Entity* Faction::spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType)
{
    if (isEntityAffordable(entityType) && !map.isPositionOccupied(position))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            m_supplyDepots.emplace_back(position);
            addedBuilding = &m_supplyDepots.back();
            increasePopulationLimit();
            break;
        case eEntityType::Barracks:
            m_barracks.emplace_back(position, *this);
            addedBuilding = &m_barracks.back();
            break;
        case eEntityType::Turret:
            m_turrets.emplace_back(position, *this);
            addedBuilding = &m_turrets.back();
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            reduceResources(entityType);
            m_allEntities.push_back(addedBuilding);
            GameEventHandler::getInstance().gameEvents.push({ eGameEventType::RevalidateMovementPaths });

            return addedBuilding;
        }
    }

    return nullptr;
}

bool Faction::addUnitToSpawn(eEntityType unitType, const Map& map, UnitSpawnerBuilding& building)
{
    if (isEntityAffordable(unitType) && !isExceedPopulationLimit(unitType))
    {
        switch (unitType)
        {
        case eEntityType::Unit:
            assert(building.getEntityType() == eEntityType::Barracks);
            static_cast<Barracks&>(building).addUnitToSpawn([this, &map, unitType](const UnitSpawnerBuilding& building)
                { return this->spawnUnit(map, building); });
            break;
        case eEntityType::Worker:
            assert(building.getEntityType() == eEntityType::HQ);
            static_cast<HQ&>(building).addUnitToSpawn([this, &map, unitType](const UnitSpawnerBuilding& building)
                { return this->spawnWorker(map, building); });
            break;
        default:
            assert(false);
        }

        return true;
    }

    return false;
}

void Faction::reduceResources(eEntityType addedEntityType)
{
    assert(isEntityAffordable(addedEntityType));
    switch (addedEntityType)
    {
    case eEntityType::Unit:
        m_currentResourceAmount -= Globals::UNIT_RESOURCE_COST;
        break;
    case eEntityType::Worker:
        m_currentResourceAmount -= Globals::WORKER_RESOURCE_COST;
        break;
    case eEntityType::SupplyDepot:
        m_currentResourceAmount -= Globals::SUPPLY_DEPOT_RESOURCE_COST;
        break;
    case eEntityType::Barracks:
        m_currentResourceAmount -= Globals::BARRACKS_RESOURCE_COST;
        break;
    case eEntityType::Turret:
        m_currentResourceAmount -= Globals::TURRET_RESOURCE_COST;
        break;
    default:
        assert(false);
    }
}

void Faction::increaseCurrentPopulationAmount(eEntityType entityType)
{
    assert(!isExceedPopulationLimit(entityType));
    switch (entityType)
    {
    case eEntityType::Unit:
        m_currentPopulationAmount += Globals::UNIT_POPULATION_COST;
        break;
    case eEntityType::Worker:
        m_currentPopulationAmount += Globals::WORKER_POPULATION_COST;
        break;
    default:
        assert(false);
    }
}

void Faction::decreaseCurrentPopulationAmount(const Entity& entity)
{
    assert(entity.isDead());
    switch (entity.getEntityType())
    {
    case eEntityType::Unit:
        m_currentPopulationAmount -= Globals::UNIT_POPULATION_COST;
        break;
    case eEntityType::Worker:
        m_currentPopulationAmount -= Globals::WORKER_POPULATION_COST;
        break;
    default:
        assert(false);
    }
}

void Faction::increasePopulationLimit()
{
    m_currentPopulationLimit += Globals::POPULATION_INCREMENT;
}

void Faction::revalidateExistingUnitPaths(const Map& map)
{
    for (auto& unit : m_units)
    {
        if (!unit.isPathEmpty())
        {
            glm::vec3 destination = unit.getDestination();
            unit.moveTo(destination, map, [&](const glm::ivec2& position)
                { return getAdjacentPositions(position, map, m_units, unit); }, unit.getCurrentState());
        }
    }

    for (auto& worker : m_workers)
    {
        if (!worker.isPathEmpty())
        {
            glm::vec3 destination = worker.getDestination();
            worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, worker.getCurrentState());
        }
    }
}

bool Faction::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker)
{
    assert(map.isWithinBounds(position) && !map.isPositionOccupied(position));

    bool withinMapBounds = false;
    switch (entityType)
    {
    case eEntityType::Barracks:
        withinMapBounds = map.isWithinBounds(AABB(position,  ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME) ));
        break;
    case eEntityType::SupplyDepot:
        withinMapBounds = map.isWithinBounds(AABB(position, ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME)));
        break;
    case eEntityType::Turret:
        withinMapBounds = map.isWithinBounds(AABB(position, ModelManager::getInstance().getModel(TURRET_MODEL_NAME)));
        break;  
    default:
        assert(false);
    }

    if (withinMapBounds)
    {
        glm::vec3 buildPosition = Globals::convertToNodePosition(position);
        auto plannedBuilding = std::find_if(m_plannedBuildings.cbegin(), m_plannedBuildings.cend(), [&buildPosition](const auto& plannedBuilding)
        {
            return plannedBuilding.getPosition() == Globals::convertToMiddleGridPosition(buildPosition);
        });

        if (plannedBuilding == m_plannedBuildings.cend())
        {
            if (worker.build([this, &map, buildPosition, entityType]()
            { return spawnBuilding(map, buildPosition, entityType); }, buildPosition, map))
            {
                m_plannedBuildings.emplace_back(worker.getID(), buildPosition, entityType);
                return true;
            }
        }
    }

    return false;
}

const Entity* Faction::spawnUnit(const Map& map, const UnitSpawnerBuilding& building)
{
    if (isEntityAffordable(eEntityType::Unit) && !isExceedPopulationLimit(eEntityType::Unit))
    {
        if (building.isWaypointActive())
        {
            m_units.emplace_back(*this, Globals::convertToNodePosition(building.getUnitSpawnPosition()), PathFindingLocator::get().getClosestAvailablePosition(
                building.getWaypointPosition(), m_units, m_workers, map), map);
        }
        else
        {
            m_units.emplace_back(*this, Globals::convertToNodePosition(PathFindingLocator::get().getClosestAvailablePosition(building.getUnitSpawnPosition(),
                m_units, m_workers, map)));
        }

        reduceResources(eEntityType::Unit);
        increaseCurrentPopulationAmount(eEntityType::Unit);
        m_allEntities.push_back(&m_units.back());

        return &m_units.back();
    }

    return nullptr;
}

const Entity* Faction::spawnWorker(const Map& map, const UnitSpawnerBuilding& building)
{
    if (isEntityAffordable(eEntityType::Worker) && !isExceedPopulationLimit(eEntityType::Worker))
    {
        if (building.isWaypointActive())
        {
            m_workers.emplace_back(*this, building.getUnitSpawnPosition(), PathFindingLocator::get().getClosestAvailablePosition(
                building.getWaypointPosition(), m_units, m_workers, map), map);
        }
        else
        {
            m_workers.emplace_back(*this, PathFindingLocator::get().getClosestAvailablePosition(
                building.getUnitSpawnPosition(), m_units, m_workers, map));
        }

        reduceResources(eEntityType::Worker);
        increaseCurrentPopulationAmount(eEntityType::Worker);
        m_allEntities.push_back(&m_workers.back());

        return &m_workers.back();
    }

    return nullptr;
}