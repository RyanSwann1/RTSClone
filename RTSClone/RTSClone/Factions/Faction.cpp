#include "Faction.h"
#include "ModelManager.h"
#include "GameEvents.h"
#include "TypeComparison.h"
#include "FactionHandler.h"
#include "Level.h"
#include "GameMessages.h"
#include "GameMessenger.h"

namespace
{
    constexpr size_t MAX_ENTITIES =
        Globals::MAX_UNITS +
        Globals::MAX_WORKERS +
        Globals::MAX_HEADQUARTERS +
        Globals::MAX_SUPPLY_DEPOTS +
        Globals::MAX_BARRACKS +
        Globals::MAX_TURRETS +
        Globals::MAX_LABORATORIES;
};

Faction::Faction(eFactionController factionController, const glm::vec3& hqStartingPosition,
    int startingResources, int startingPopulationCap)
    : m_controller(factionController),
    m_currentResourceAmount(startingResources),
    m_currentPopulationLimit(startingPopulationCap)
{
    m_allEntities.reserve(MAX_ENTITIES);
    m_units.reserve(Globals::MAX_UNITS);
    m_workers.reserve(Globals::MAX_WORKERS);
    m_headquarters.reserve(Globals::MAX_HEADQUARTERS);
    m_supplyDepots.reserve(Globals::MAX_SUPPLY_DEPOTS);
    m_barracks.reserve(Globals::MAX_BARRACKS);
    m_turrets.reserve(Globals::MAX_TURRETS);

    Entity& entity = m_headquarters.emplace_back(hqStartingPosition, *this);
    m_allEntities.push_back(&entity);
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

const Headquarters* Faction::getMainHeadquarters() const
{
    return !m_headquarters.empty() ? &m_headquarters.front() : nullptr;
}

const Headquarters* Faction::getClosestHeadquarters(const glm::vec3& position) const
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

	return closestHeadquarters;
}

eFactionController Faction::getController() const
{
    return m_controller;
}

const std::vector<Entity*>& Faction::getEntities() const
{
    return m_allEntities;
}

const Entity* Faction::getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits) const
{
    const Entity* closestEntity = nullptr;
    if (prioritizeUnits)
    {
        float closestEntityDistance = maxDistance * maxDistance;
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (!closestEntity && distance < closestEntityDistance)
            {
                closestEntity = &*entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && Globals::BUILDING_TYPES.isMatch(closestEntity->getEntityType()) &&
                Globals::UNIT_TYPES.isMatch(entity->getEntityType()) &&
                Globals::getSqrDistance(entity->getPosition(), position) < maxDistance * maxDistance)
            {
                closestEntity = &*entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && distance < closestEntityDistance)
            {
                closestEntity = &*entity;
                closestEntityDistance = distance;
            }
        }
    }
    else
    {
        float closestEntityDistance = maxDistance * maxDistance;
        for (const auto& entity : m_allEntities)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (distance < closestEntityDistance)
            {
                closestEntity = &*entity;
                closestEntityDistance = distance;
            }
        }
    }

    return closestEntity;
}

const Entity* Faction::getEntity(const AABB& aabb, int entityID) const
{
    const auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&aabb, entityID](auto entity)
    {
        return entity->getID() == entityID && entity->getAABB().contains(aabb);
    });

    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }
    return nullptr;
}

const Entity* Faction::getEntity(const glm::vec3& position) const
{
    const auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }
    return nullptr;
}

const Entity* Faction::getEntity(int entityID) const
{
    const auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [entityID](auto entity)
    {
        return entity->getID() == entityID;
    });
    if (entity != m_allEntities.cend())
    {
        return (*entity);
    }
    return nullptr;
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    switch (gameEvent.type)
    {
    case eGameEventType::TakeDamage:
    {
        assert(gameEvent.data.takeDamage.senderFaction != m_controller);
        const auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [id = gameEvent.data.takeDamage.targetID](const auto& entity)
        {
            return entity->getID() == id;
        });
        if (entity == m_allEntities.end())
        {
            break;
        }

        (*entity)->takeDamage(gameEvent.data.takeDamage, map);
        if (!(*entity)->isDead())
        {
            on_entity_taken_damage(gameEvent.data.takeDamage, *(*entity), map, factionHandler);
            break;
        }
        switch ((*entity)->getEntityType())
        {
        case eEntityType::Worker:
            removeEntity<Worker>(m_workers, entity);
            break;
        case eEntityType::Unit:
            removeEntity<Unit>(m_units, entity);
            break;
        case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot>(m_supplyDepots, entity);
            break;
        case eEntityType::Barracks:
            removeEntity<Barracks>(m_barracks, entity);
            break;
        case eEntityType::Headquarters:
            removeEntity<Headquarters>(m_headquarters, entity);
            if (m_headquarters.empty())
            {
                Level::add_event(GameEvent::create<EliminateFactionEvent>({}));
            }
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_turrets, entity);
            break;
        case eEntityType::Laboratory:
            m_laboratory = std::nullopt;
            break;
        default:
            assert(false);
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        revalidateExistingUnitPaths(map);
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
            isAffordable(Globals::FACTION_SHIELD_INCREASE_COST) &&
            m_laboratory.has_value())
        {
            m_laboratory->handleEvent(gameEvent.data.increaseFactionShield);
        }
        break;
    case eGameEventType::ForceSelfDestructEntity:
    {
        const auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [id = gameEvent.data.forceSelfDestructEntity.entityID](auto& entity)
        {
            return entity->getID() == id;
        });
        if (entity == m_allEntities.end())
        {
            break;
        }

        switch (gameEvent.data.forceSelfDestructEntity.entityType)
        {
        case eEntityType::Worker:
            removeEntity<Worker>(m_workers, entity);
            break;
        case eEntityType::Unit:
            removeEntity<Unit>(m_units, entity);
            break;
        case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot>(m_supplyDepots, entity);
            break;
        case eEntityType::Barracks:
            removeEntity<Barracks>(m_barracks, entity);
            break;
        case eEntityType::Headquarters:
            removeEntity<Headquarters>(m_headquarters, entity);
            if (m_headquarters.empty())
            {
                Level::add_event(GameEvent::create<EliminateFactionEvent>({}));
            }
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_turrets, entity);
            break;
        case eEntityType::Laboratory:
            m_laboratory = std::nullopt;
            break;
        default:
            assert(false);
        }
        break;
    }
    case eGameEventType::EntityIdle:
    {
        const int entityID = gameEvent.data.entityIdle.entityID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [entityID](const auto& entity)
        {
            return entity->getID() == entityID;
        });
        if (entity != m_allEntities.cend())
        {
            on_entity_idle(*(*entity), map, factionHandler, baseHandler);
        }
    }
        break;
    case eGameEventType::AddFactionResources:
        m_currentResourceAmount += gameEvent.data.addFactionResources.quantity;
        break;
    }
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
        if (worker.getCurrentState() == eWorkerState::Idle)
        {
            if (map.isCollidable(worker.getPosition()))
            {
                glm::vec3 destination(0.f);
                if (PathFinding::getInstance().getClosestAvailablePosition(worker, m_workers, map, destination))
                {
                    worker.moveTo(destination, map);
                }
            }
            else
            {
                for (const auto& otherWorker : m_workers)
                {
                    if (worker.getID() != otherWorker.getID() &&
                        foundWorker(otherWorker.getID()) &&
                        otherWorker.getCurrentState() == eWorkerState::Idle &&
                        worker.getAABB().contains(otherWorker.getAABB()))
                    {
                        glm::vec3 destination(0.f);
                        if (PathFinding::getInstance().getClosestAvailablePosition(worker, m_workers, map, destination))
                        {
                            worker.moveTo(destination, map);
                            break;
                        }
                    }
                }
            }
        }

        handledWorkers.push_back(worker);
    }

    handledWorkers.clear();
}

const Headquarters* Faction::get_closest_headquarters(const glm::vec3& position) const
{
    const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_headquarters)
    {
        const float result = Globals::getSqrDistance(headquarters.getPosition(), position);
        if (result < distance)
        {
            distance = result;
            closestHeadquarters = &headquarters;
        }
    }

    return closestHeadquarters;
}

const Entity* Faction::get_entity(const int id) const
{
    auto entity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [id](const auto& entity)
    {
        return entity->getID() == id;
    });
    if (entity != m_allEntities.cend())
    {
        return &*(*entity);
    }
    return nullptr;
}

Entity* Faction::create_building(const Worker& worker, const Map& map)
{
    assert(worker.getCurrentState() == eWorkerState::Building && !worker.get_scheduled_buildings().empty());

    eEntityType entityType = worker.get_scheduled_buildings().front().entityType;
    const glm::vec3& position = worker.get_scheduled_buildings().front().position;
    if (isAffordable(entityType) && !map.isPositionOccupied(position))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            if (m_supplyDepots.size() < Globals::MAX_SUPPLY_DEPOTS)
            {
                addedBuilding = &m_supplyDepots.emplace_back(position, *this);
                increasePopulationLimit();
            }
            break;
        case eEntityType::Barracks:
            if (m_barracks.size() < Globals::MAX_BARRACKS)
            {
                addedBuilding = &m_barracks.emplace_back(position, *this);
            }
            break;
        case eEntityType::Turret:
            if (m_turrets.size() < Globals::MAX_TURRETS)
            {
                addedBuilding = &m_turrets.emplace_back(position, *this);
            }
            break;
        case eEntityType::Headquarters:
            if (m_headquarters.size() < Globals::MAX_HEADQUARTERS)
            {
                addedBuilding = &m_headquarters.emplace_back(position, *this);
            }
            break;
        case eEntityType::Laboratory:
            if (!m_laboratory.has_value())
            {
                addedBuilding = &m_laboratory.emplace(position, *this);
            }
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            m_allEntities.push_back(addedBuilding);
            reduceResources(entityType);
            Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));

            return addedBuilding;
        }
    }

    return nullptr;
}

void Faction::update(float deltaTime, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    for (auto& unit : m_units)
    {
        unit.update(deltaTime, factionHandler, map);
    }

    for (auto& worker : m_workers)
    {
        worker.update(deltaTime, map, factionHandler);
    }

    for (auto& barracks : m_barracks)
    {
        barracks.update(deltaTime, *this, map);
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
        headquarters.update(deltaTime, *this, map);
    }

    if (m_laboratory)
    {
        m_laboratory->update(deltaTime);
    }
}

void Faction::delayed_update(const Map& map, const FactionHandler& factionHandler)
{
    for (auto& unit : m_units)
    {
        unit.delayed_update(factionHandler, map);
    }

    for (auto& worker : m_workers)
    {
        worker.delayed_update(map, factionHandler);
    }

    handleWorkerCollisions(map);
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
        barracks.render_waypoint(shaderHandler);
    }

    for (const auto& turret : m_turrets)
    {
        turret.render(shaderHandler, m_controller);
    }

    for (const auto& headquarters : m_headquarters)
    {
        headquarters.render(shaderHandler, m_controller);
        headquarters.render_waypoint(shaderHandler);
    }
 
    if (m_laboratory)
    {
        m_laboratory->render(shaderHandler, m_controller);
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
            static_cast<Barracks&>(*entity).render_progress_bar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Headquarters:
            static_cast<Headquarters&>(*entity).render_progress_bar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Worker:
            static_cast<Worker&>(*entity).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Laboratory:
            static_cast<Laboratory&>(*entity).renderProgressBar(shaderHandler, camera, windowSize);
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
        unit.render_path(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker.render_path(shaderHandler);
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
        auto buildingCommand = std::find_if(worker.get_scheduled_buildings().cbegin(), worker.get_scheduled_buildings().cend(),
            [&AABB](const auto& buildingCommand)
        {
            return AABB.contains(buildingCommand.position);
        });
        if (buildingCommand != worker.get_scheduled_buildings().cend())
        {
            return true;
        }
    }

    return false;
}

bool Faction::isBuildingInAllWorkersQueue(eEntityType entityType) const
{
    return std::find_if(m_workers.cbegin(), m_workers.cend(), [entityType](auto& worker)
    {
        return worker.isInBuildQueue(entityType);
    }) != m_workers.cend();
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
            entity->increaseMaximumShield(*this);
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

void Faction::revalidateExistingUnitPaths(const Map& map)
{
    for (auto& unit : m_units)
    {
        unit.revalidate_movement_path(map);
    }

    for (auto& worker : m_workers)
    {
        worker.revalidate_movement_path(map);
    }
}

bool Faction::is_laboratory_built() const
{
    return m_laboratory.has_value();
}

bool Faction::isMineralInUse(const Mineral& mineral) const
{
    return std::any_of(m_workers.cbegin(), m_workers.cend(), [&mineral](auto& worker)
    {
        return worker.getMineralToHarvest() && worker.getMineralToHarvest()->getPosition() == mineral.getPosition();
    });
}

Entity* Faction::createUnit(const Map& map, const EntitySpawnerBuilding& spawner)
{
    return create_entity<Unit>(map, spawner, eEntityType::Unit, m_units, Globals::MAX_UNITS);
}

Entity* Faction::createWorker(const Map& map, const EntitySpawnerBuilding& spawner)
{
    return create_entity(map, spawner, eEntityType::Worker, m_workers, Globals::MAX_WORKERS);
}