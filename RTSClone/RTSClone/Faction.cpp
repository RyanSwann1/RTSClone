#include "Faction.h"
#include "GameEventHandler.h"
#include "ModelManager.h"
#include "GameEvents.h"
#include "TypeComparison.h"
#include "FactionHandler.h"

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
    m_headquarters.emplace_back(hqStartingPosition, *this);
    m_allEntities.push_back(&m_headquarters.back());
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

const Headquarters& Faction::getClosestHeadquarters(const glm::vec3& position) const
{
    const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_headquarters)
    {
        float result = Globals::getSqrDistance(headquarters.getPosition(), position);
        if (result < distance)
        {
            distance = result;
            closestHeadquarters = &headquarters;
        }
    }

    assert(closestHeadquarters);
    return *closestHeadquarters;
}

const glm::vec3& Faction::getMainHeadquartersPosition() const
{
    assert(!m_headquarters.empty());
    return m_headquarters.begin()->getPosition();
}

eFactionController Faction::getController() const
{
    return m_controller;
}

const std::list<Unit>& Faction::getUnits() const
{
    return m_units;
}

const std::list<Worker>& Faction::getWorkers() const
{
    return m_workers;
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
            return entity->getID() == targetID;
        });

        if (entity != m_allEntities.end())
        {
            switch ((*entity)->getEntityType())
            {
            case eEntityType::Worker:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
                {
                    removeEntity<Worker>(m_workers, targetID, entity);
                }
                break;
            case eEntityType::Unit:
                static_cast<Unit&>(*(*entity)).reduceHealth(gameEvent.data.takeDamage, factionHandler, map);
                if ((*entity)->isDead())
                {
                    removeEntity<Unit>(m_units, targetID, entity);
                }
                break;
            case eEntityType::SupplyDepot:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
                {
                    removeEntity<SupplyDepot>(m_supplyDepots, targetID, entity);
                }
                break;
            case eEntityType::Barracks:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
                {
                    removeEntity<Barracks>(m_barracks, targetID, entity);
                }
                break;
            case eEntityType::Headquarters:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
                {
                    removeEntity<Headquarters>(m_headquarters, targetID, entity);
                    if (m_headquarters.empty())
                    {
                        GameEventHandler::getInstance().gameEvents.push(GameEvent::createEliminateFaction(m_controller));
                    }
                }
                break;
            case eEntityType::Turret:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
                {
                    removeEntity<Turret>(m_turrets, targetID, entity);
                }
                break;
            case eEntityType::Laboratory:
                (*entity)->reduceHealth(gameEvent.data.takeDamage);
                if ((*entity)->isDead())
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
            return entity->getID() == entityID;
        });
        if (entity != m_allEntities.end())
        {
            (*entity)->repair();
        }
    }
        break;
    case eGameEventType::IncreaseFactionShield:
        if (m_currentShieldAmount < Globals::MAX_FACTION_SHIELD_AMOUNT &&
            isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
        {
            increaseShield();
        }
        break;
    }
}

void Faction::addResources(Worker& worker)
{
    m_currentResourceAmount += worker.extractResources();
}

void Faction::increaseShield()
{
    assert(m_laboratories.size() == 1 && m_currentShieldAmount < Globals::MAX_FACTION_SHIELD_AMOUNT);
    if (isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
    {
        std::vector<Entity*>& allEntities = m_allEntities;
        int& currentResourcesAmount = m_currentResourceAmount;
        int& currentShieldAmount = m_currentShieldAmount;
        m_laboratories.front().addIncreaseShieldCommand([&allEntities, &currentResourcesAmount, &currentShieldAmount, this]()
        {
            if (this->isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
            {
                ++currentShieldAmount;
                currentResourcesAmount -= Globals::FACTION_SHIELD_INCREASE_COST;

                for (auto& entity : allEntities)
                {
                    entity->increaseMaximumShield(*this);
                }
            }
        });
    }
}

void Faction::handleUnitCollisions(const Map& map, FactionHandler& factionHandler)
{
    std::vector<const Entity*> handledUnits;
    for (auto& unit : m_units)
    {
        if (unit.getCurrentState() == eUnitState::Idle)
        {
            if (map.isCollidable(unit.getPosition()))
            {
                unit.moveTo(PathFinding::getInstance().getClosestAvailablePosition<Unit>(unit, m_units, map), map,
                    [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, factionHandler, unit); }, factionHandler);
            }
            else
            {
                for (const auto& otherUnit : m_units)
                {
                    if (&unit != &otherUnit &&
                        std::find(handledUnits.cbegin(), handledUnits.cend(), &otherUnit) == handledUnits.cend() &&
                        otherUnit.getCurrentState() == eUnitState::Idle &&
                        unit.getAABB().contains(otherUnit.getAABB()))
                    {
                        unit.moveTo(PathFinding::getInstance().getClosestAvailablePosition<Unit>(unit, m_units, map), map,
                            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, factionHandler, unit); }, factionHandler);
                        break;
                    }
                }
            }
        }

        handledUnits.push_back(&unit);
    }

    handledUnits.clear();
}

void Faction::handleWorkerCollisions(const Map& map)
{
    std::vector<const Entity*> handledUnits;
    for (auto& worker : m_workers)
    {
        if (worker.getCurrentState() == eWorkerState::Idle)
        {
            if (map.isCollidable(worker.getPosition()))
            {
                worker.moveTo(PathFinding::getInstance().getClosestAvailablePosition<Worker>(worker, m_workers, map), map,
                    [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
            }
            else
            {
                for (const auto& otherWorker : m_workers)
                {
                    if (&worker != &otherWorker &&
                        std::find(handledUnits.cbegin(), handledUnits.cend(), &otherWorker) == handledUnits.cend() &&
                        otherWorker.getCurrentState() == eWorkerState::Idle &&
                        worker.getAABB().contains(otherWorker.getAABB()))
                    {
                        worker.moveTo(PathFinding::getInstance().getClosestAvailablePosition<Worker>(worker, m_workers, map), map,
                            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
                        break;
                    }
                }
            }
        }

        handledUnits.push_back(&worker);
    }

    handledUnits.clear();
}

void Faction::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime, factionHandler, map, unitStateHandlerTimer);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, map, factionHandler, unitStateHandlerTimer);
    }

    for (auto& barracks : m_barracks)
    {
        barracks.update(deltaTime, map, factionHandler);
    }

    for (auto& turret : m_turrets)
    {
        turret.update(deltaTime, factionHandler, map);
    }

    for (auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.update(deltaTime);
    }

    for (auto& headquarters : m_headquarters)
    {
        headquarters.update(deltaTime, map, factionHandler);
    }

    for (auto& laboratory : m_laboratories)
    {
        laboratory.update(deltaTime);
    }

    if (unitStateHandlerTimer.isExpired())
    {
        handleUnitCollisions(map, factionHandler);
        handleWorkerCollisions(map);
    }
}

void Faction::render(ShaderHandler& shaderHandler) const
{
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

    for (const auto& turret : m_turrets)
    {
        turret.render(shaderHandler, m_controller);
    }

    for (const auto& headquarters : m_headquarters)
    {
        headquarters.render(shaderHandler, m_controller);
    }
 
    for (const auto& laboratory : m_laboratories)
    {
        laboratory.render(shaderHandler, m_controller);
    }
}

void Faction::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
    for (const auto& worker : m_workers)
    {
        worker.renderBuildingCommands(shaderHandler);
    }
}

void Faction::renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
    for (const auto& entity : m_allEntities)
    {
        entity->renderHealthBar(shaderHandler, camera, windowSize);
        entity->renderShieldBar(shaderHandler, camera, windowSize);

        switch (entity->getEntityType())
        {
        case eEntityType::Barracks:
            static_cast<Barracks&>(*(entity)).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Headquarters:
            static_cast<Headquarters&>(*(entity)).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Worker:
            static_cast<Worker&>(*(entity)).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Laboratory:
            static_cast<Laboratory&>(*(entity)).renderProgressBar(shaderHandler, camera, windowSize);
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

const Entity* Faction::spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType)
{
    if (isAffordable(entityType) && !map.isPositionOccupied(position))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            m_supplyDepots.emplace_back(position, *this);
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
        case eEntityType::Headquarters:
            m_headquarters.emplace_back(position, *this);
            addedBuilding = &m_headquarters.back();
            break;
        case eEntityType::Laboratory:
            m_laboratories.emplace_back(position, *this);
            addedBuilding = &m_laboratories.back();
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            reduceResources(entityType);
            m_allEntities.push_back(addedBuilding);
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createRevalidateMovementPaths());

            return addedBuilding;
        }
    }

    return nullptr;
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
        if (!unit.getPathToPosition().empty())
        {
            unit.moveTo(unit.getPathToPosition().front(), map, [&](const glm::ivec2& position)
                { return getAdjacentPositions(position, map, factionHandler, unit); }, factionHandler, unit.getCurrentState());
        }
    }

    for (auto& worker : m_workers)
    {
        if (!worker.getPathToPosition().empty())
        {
            worker.moveTo(worker.getPathToPosition().front(), map,
                [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); }, worker.getCurrentState(),
                worker.getMineralToHarvest());
        }
    }
}

bool Faction::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker)
{
    assert(map.isWithinBounds(position) && !map.isPositionOccupied(position));

    AABB buildingAABB = ModelManager::getInstance().getModelAABB(position, entityType);
    if (map.isWithinBounds(buildingAABB))
    {
        bool buildingCommandCollision = false;
        for (const auto& worker : m_workers)
        {
            auto buildingCommand = std::find_if(worker.getBuildingCommands().cbegin(), worker.getBuildingCommands().cend(),
                [&buildingAABB, &position](const auto& buildingCommand)
            {
                return buildingAABB.contains(buildingCommand.position);
            });
            if (buildingCommand != worker.getBuildingCommands().cend())
            {
                buildingCommandCollision = true;
                break;
            }
        }

        if (!buildingCommandCollision)
        {
            return worker.build(position, map, entityType);
        }
    }

    return false;
}

const Entity* Faction::spawnUnit(const Map& map, const EntitySpawnerBuilding& building, FactionHandler& factionHandler)
{
    if (isAffordable(eEntityType::Unit) && !isExceedPopulationLimit(eEntityType::Unit))
    {
        if (building.isWaypointActive())
        {
            glm::vec3 startingPosition = building.getUnitSpawnPosition();
            glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, building.getPosition()), 0.0f };
            glm::vec3 destination = PathFinding::getInstance().getClosestAvailablePosition(building.getWaypointPosition(), m_units, m_workers, map);

            m_units.emplace_back(*this, startingPosition, startingRotation, destination, factionHandler, map);
        }
        else
        {
            glm::vec3 startingPosition = PathFinding::getInstance().getClosestAvailablePosition(building.getUnitSpawnPosition(),
                m_units, m_workers, map);
            glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, building.getPosition()), 0.0f };
            
            m_units.emplace_back(*this, startingPosition, startingRotation);
        }

        reduceResources(eEntityType::Unit);
        increaseCurrentPopulationAmount(eEntityType::Unit);
        m_allEntities.push_back(&m_units.back());

        return &m_units.back();
    }

    return nullptr;
}

const Entity* Faction::spawnWorker(const Map& map, const EntitySpawnerBuilding& building)
{
    if (isAffordable(eEntityType::Worker) && !isExceedPopulationLimit(eEntityType::Worker))
    {
        if (building.isWaypointActive())
        {
            glm::vec3 startingPosition = building.getUnitSpawnPosition();
            glm::vec3 destination = PathFinding::getInstance().getClosestAvailablePosition(
                building.getWaypointPosition(), m_units, m_workers, map);

            m_workers.emplace_back(*this, startingPosition, destination, map);
        }
        else
        {
            glm::vec3 startingPosition = PathFinding::getInstance().getClosestAvailablePosition(
                building.getUnitSpawnPosition(), m_units, m_workers, map);
            glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, building.getPosition()), 0.0f };

            m_workers.emplace_back(*this, startingPosition, startingRotation);
        }

        reduceResources(eEntityType::Worker);
        increaseCurrentPopulationAmount(eEntityType::Worker);
        m_allEntities.push_back(&m_workers.back());

        return &m_workers.back();
    }

    return nullptr;
}