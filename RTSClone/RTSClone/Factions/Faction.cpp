#include "Faction.h"
#include "Graphics/ModelManager.h"
#include "Events/GameEvents.h"
#include "Core/TypeComparison.h"
#include "FactionHandler.h"
#include "Core/Level.h"
#include "Events/GameMessages.h"
#include "Events/GameMessenger.h"

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

    Entity& entity = m_headquarters.emplace_back(Position{ hqStartingPosition, GridLockActive::True }, *this);
    m_allEntities.push_back(&entity);
}

void Faction::on_entity_removal(const Entity& entity)
{
    assert(entity.isDead());
    m_currentPopulationAmount -= Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entity.getEntityType())];
}

Worker* Faction::GetWorker(const int id)
{
    const auto worker = std::find_if(m_workers.begin(), m_workers.end(), [id](const auto& worker)
    {
        return worker.getID() == id;
    });
    if (worker != m_workers.end())
    {
        return &(*worker);
    }

    return nullptr;
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

int Faction::get_headquarters_count() const
{
    return m_headquarters.size();
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

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler)
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
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_turrets, entity);
            break;
        case eEntityType::Laboratory:
            removeEntity<Laboratory>(m_laboratories, entity);
            break;
        default:
            assert(false);
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        for (auto& unit : m_units)
        {
            unit.revalidate_movement_path(map);
        }

        for (auto& worker : m_workers)
        {
            worker.revalidate_movement_path(map);
        }
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
            !m_laboratories.empty())
        {
            for (auto& laboratory : m_laboratories)
            {
                laboratory.handleEvent(gameEvent.data.increaseFactionShield);
            }
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
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_turrets, entity);
            break;
        case eEntityType::Laboratory:
            removeEntity<Laboratory>(m_laboratories, entity);
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
                    worker.MoveTo(destination, map, false);
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
                            worker.MoveTo(destination, map, false);
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

void Faction::on_entity_creation(Entity& entity)
{
    m_currentResourceAmount -= Globals::ENTITY_RESOURCE_COSTS[static_cast<int>(entity.getEntityType())];
    m_currentPopulationAmount += Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entity.getEntityType())];
    if (entity.getEntityType() == eEntityType::SupplyDepot)
    {
        m_currentPopulationLimit += Globals::POPULATION_INCREMENT;
    }

    m_allEntities.push_back(&entity);
}

bool Faction::is_entity_creatable(eEntityType type, const size_t current, const size_t max) const
{
    return current < max
        && isAffordable(type)
        && !isExceedPopulationLimit(type);
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

Barracks* Faction::CreateBarracks(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateBuilding(m_barracks, scheduled_building, Globals::MAX_BARRACKS);
}

Turret* Faction::CreateTurret(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateBuilding(m_turrets, scheduled_building, Globals::MAX_TURRETS);
}

Headquarters* Faction::CreateHeadquarters(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateBuilding(m_headquarters, scheduled_building, Globals::MAX_HEADQUARTERS);
}

Laboratory* Faction::CreateLaboratory(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateBuilding(m_laboratories, scheduled_building, Globals::MAX_LABORATORIES);
}

SupplyDepot* Faction::CreateSupplyDepot(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateBuilding(m_supplyDepots, scheduled_building, Globals::MAX_SUPPLY_DEPOTS);
}

void Faction::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler)
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

    for (auto& laboratory : m_laboratories)
    {
        laboratory.update(deltaTime);
    }
}

void Faction::delayed_update(const Map& map, FactionHandler& factionHandler)
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
        entity->render_status_bars(shaderHandler, camera, windowSize);
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
        if (worker.is_colliding_with_scheduled_buildings(AABB))
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

bool Faction::is_laboratory_built() const
{
    return !m_laboratories.empty();
}

bool Faction::isMineralInUse(const Mineral& mineral) const
{
    return std::any_of(m_workers.cbegin(), m_workers.cend(), [&mineral](auto& worker)
    {
        return worker.getMineralToHarvest() && worker.getMineralToHarvest()->getPosition() == mineral.getPosition();
    });
}

Entity* Faction::createUnit(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map)
{
    return CreateEntityFromBuilding(map, entity_to_spawn, m_units, Globals::MAX_UNITS);
}

Entity* Faction::createWorker(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map)
{
    return CreateEntityFromBuilding(map, entity_to_spawn, m_workers, Globals::MAX_WORKERS);
}