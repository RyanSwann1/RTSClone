#include "Faction.h"
#include "GameEventHandler.h"
#include "ModelManager.h"
#include "GameEvent.h"
#include "TypeComparison.h"

namespace
{
    const TypeComparison<eEntityType> BUILDING_TYPES =
    {
        {eEntityType::HQ,
        eEntityType::SupplyDepot,
        eEntityType::Barracks}
    }; 

    const TypeComparison<eEntityType> UNIT_TYPES =
    {
        {eEntityType::Unit,
        eEntityType::Worker}
    };
}

//PlannedBuilding
PlannedBuilding::PlannedBuilding(int workerID, const glm::vec3& spawnPosition, eEntityType entityType)
    : workerID(workerID),
    spawnPosition(Globals::convertToMiddleGridPosition(spawnPosition)),
    entityType(entityType)
{
    assert(entityType == eEntityType::Barracks || entityType == eEntityType::SupplyDepot);
}

//Faction
Faction::Faction(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition)
    : m_plannedBuildings(),
    m_minerals(),
    m_allEntities(),
    m_units(),
    m_workers(),
    m_supplyDepots(),
    m_barracks(),
    m_HQ(Globals::convertToNodePosition(hqStartingPosition), eEntityType::HQ),
    m_factionName(factionName),
    m_currentResourceAmount(Globals::STARTING_RESOURCES),
    m_currentPopulationAmount(0),
    m_currentPopulationLimit(Globals::STARTING_POPULATION)
{
    m_allEntities.push_back(&m_HQ);

    m_minerals.reserve(Globals::MAX_MINERALS_PER_FACTION);
    glm::vec3 startingPosition = mineralsStartingPosition;
    for (int i = 0; i < Globals::MAX_MINERALS_PER_FACTION; ++i)
    {
        glm::vec3 position = mineralsStartingPosition;
        position.z += Globals::NODE_SIZE * i;
        m_minerals.emplace_back(Globals::convertToNodePosition(position));
    }
}

eFactionName Faction::getName() const
{
    return m_factionName;
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
            else if (closestEntity && BUILDING_TYPES.isMatch(closestEntity->getEntityType()) && UNIT_TYPES.isMatch(entity->getEntityType()) &&
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

int Faction::getEntityIDAtPosition(const glm::vec3& position) const
{
    auto cIter = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });

    if (cIter != m_allEntities.cend())
    {
        return (*cIter)->getID();
    }
    else
    {
        return Globals::INVALID_ENTITY_ID;
    }
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map)
{
    switch (gameEvent.type)
    {
    case eGameEventType::Attack:
    {
        assert(gameEvent.senderFaction != m_factionName);
        int targetID = gameEvent.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetID](const auto& entity)
        {
            return entity->getID() == targetID;
        });
        if (entity != m_allEntities.end())
        {
            switch ((*entity)->getEntityType())
            {
            case eEntityType::Worker:
            {
                auto worker = std::find_if(m_workers.begin(), m_workers.end(), [targetID](const auto& worker)
                {
                    return worker.getID() == targetID;
                });
                assert(worker != m_workers.end());

                m_workers.erase(worker);
                m_allEntities.erase(entity);
            }
            break;
            }
        }
    }
        break;
    case eGameEventType::RemovePlannedBuilding:
    {
        assert(gameEvent.senderFaction == m_factionName);
        const glm::vec3& buildingPosition = gameEvent.startingPosition;
        auto buildingToSpawn = std::find_if(m_plannedBuildings.begin(), m_plannedBuildings.end(), [&buildingPosition](const auto& buildingToSpawn)
        {
            return buildingToSpawn.spawnPosition == buildingPosition;
        });

        assert(buildingToSpawn != m_plannedBuildings.end());
        m_plannedBuildings.erase(buildingToSpawn);
    }
        break;
    case eGameEventType::RemoveAllWorkerPlannedBuildings:
        for (auto plannedBuilding = m_plannedBuildings.begin(); plannedBuilding != m_plannedBuildings.end();)
        {
            if (plannedBuilding->workerID == gameEvent.senderID)
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
    default:
        assert(false);
    }
}

void Faction::addResources(Worker& worker)
{
    m_currentResourceAmount += worker.extractResources();
}

void Faction::update(float deltaTime, const Map& map, const Faction& opposingFaction)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime, opposingFaction, map, m_units);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, m_HQ, map, opposingFaction, m_units);
    }

    handleCollisions<Unit>(m_units, map);
    handleCollisions<Worker>(m_workers, map);
}

void Faction::render(ShaderHandler& shaderHandler) const
{
    m_HQ.render(shaderHandler);

    for (const auto& unit : m_units)
    {
        unit.render(shaderHandler);
    }

    for (const auto& worker : m_workers)
    {
        worker.render(shaderHandler);
    }

    for (const auto& supplyDepot : m_supplyDepots)
    {
        supplyDepot.render(shaderHandler);
    }

    for (const auto& barracks : m_barracks)
    {
        barracks.render(shaderHandler);
    }

    for (const auto& minerals : m_minerals)
    {
        minerals.render(shaderHandler);
    }
}

void Faction::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
    for (const auto& plannedBuilding : m_plannedBuildings)
    {
        ModelManager::getInstance().getModel(plannedBuilding.entityType).render(shaderHandler, plannedBuilding.spawnPosition);
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
    default:
        assert(false);
        return false;
    }
}

const Entity* Faction::addBuilding(Worker& worker, const Map& map, glm::vec3 spawnPosition, eEntityType entityType)
{
    if (isEntityAffordable(entityType) &&
        PathFinding::getInstance().isPositionAvailable(spawnPosition, map, m_units, m_workers, worker))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            if (m_currentPopulationLimit + Globals::POPULATION_INCREMENT <= Globals::MAX_POPULATION)
            {
                m_supplyDepots.emplace_back(spawnPosition);
                addedBuilding = &m_supplyDepots.back();
                increasePopulationLimit();
            }
            break;
        case eEntityType::Barracks:
            m_barracks.emplace_back(spawnPosition, eEntityType::Barracks);
            addedBuilding = &m_barracks.back();
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            reduceResources(entityType);
            revalidateExistingUnitPaths(map);
            m_allEntities.push_back(addedBuilding);
            GameEventHandler::getInstance().addEvent({ eGameEventType::RevalidateMovementPaths });

            return addedBuilding;
        }
    }

    return nullptr;
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

void Faction::increasePopulationLimit()
{
    assert(m_currentPopulationLimit + Globals::POPULATION_INCREMENT <= Globals::MAX_POPULATION);
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
            { return getAllAdjacentPositions(position, map, m_units, unit); }, unit.getCurrentState());
        }
    }

    for (auto& worker : m_workers)
    {
        if (!worker.isPathEmpty())
        {
            glm::vec3 destination = worker.getDestination();
            worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); }, worker.getCurrentState());
        }
    }
}

void Faction::instructWorkerToBuild(eEntityType entityType, const glm::vec3& mouseToGroundPosition, const Map& map)
{
    if (!Globals::isPositionInMapBounds(mouseToGroundPosition) || map.isPositionOccupied(mouseToGroundPosition))
    {
        return;
    }

    switch (entityType)
    {
    case eEntityType::Barracks:
    case eEntityType::SupplyDepot:
    {
        glm::vec3 buildPosition = Globals::convertToNodePosition(mouseToGroundPosition);
        auto plannedBuilding = std::find_if(m_plannedBuildings.cbegin(), m_plannedBuildings.cend(), [&buildPosition](const auto& plannedBuilding)
        {
            return plannedBuilding.spawnPosition == Globals::convertToMiddleGridPosition(buildPosition);
        });

        if (plannedBuilding == m_plannedBuildings.cend())
        {
            auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [](const auto& worker)
            {
                return worker.isSelected();
            });
            if (selectedWorker != m_workers.end())
            {
                if (selectedWorker->build([this, &map, buildPosition, entityType](Worker& worker)
                { return addBuilding(worker, map, buildPosition, entityType); }, buildPosition, map))
                {
                    m_plannedBuildings.emplace_back(selectedWorker->getID(), buildPosition, entityType);
                }
            }
        }
    }
    break;
    default:
        assert(false);
    }
}