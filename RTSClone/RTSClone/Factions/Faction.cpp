#include "Faction.h"
#include "Graphics/ModelManager.h"
#include "Events/GameEvents.h"
#include "Core/TypeComparison.h"
#include "FactionHandler.h"
#include "Core/Level.h"
#include "Events/GameMessages.h"
#include "Events/GameMessenger.h"
#include <numeric>

namespace
{
    constexpr std::array<size_t, static_cast<int>(eEntityType::Max) + 1> MAX_ENTITY_QUANTITIES
    {
        Globals::MAX_UNITS,
        Globals::MAX_WORKERS, 
        Globals::MAX_HEADQUARTERS,
        Globals::MAX_SUPPLY_DEPOTS,
        Globals::MAX_BARRACKS,
        Globals::MAX_TURRETS,
        Globals::MAX_LABORATORIES
    };
};

Faction::Faction(eFactionController factionController, const glm::vec3& hqStartingPosition,
    int startingResources, int startingPopulationCap)
    : m_controller(factionController),
    m_currentResourceAmount(startingResources),
    m_currentPopulationLimit(startingPopulationCap)
{
    m_entities.all.reserve(std::accumulate(MAX_ENTITY_QUANTITIES.cbegin(), MAX_ENTITY_QUANTITIES.cend(), 0));
    m_entities.units.reserve(Globals::MAX_UNITS);
    m_entities.workers.reserve(Globals::MAX_WORKERS);
    m_entities.headquarters.reserve(Globals::MAX_HEADQUARTERS);
    m_entities.supply_depots.reserve(Globals::MAX_SUPPLY_DEPOTS);
    m_entities.barracks.reserve(Globals::MAX_BARRACKS);
    m_entities.turrets.reserve(Globals::MAX_TURRETS);

    Entity& entity = m_entities.headquarters.emplace_back(Position{ hqStartingPosition, GridLockActive::True }, *this);
    m_entities.all.push_back(&entity);
}

void Faction::on_entity_removal(const Entity& entity)
{
    assert(entity.isDead());
    m_currentPopulationAmount -= Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entity.getEntityType())];
}

Worker* Faction::GetWorker(const int id)
{
    const auto worker = std::find_if(m_entities.workers.begin(), m_entities.workers.end(), [id](const auto& worker)
    {
        return worker.getID() == id;
    });
    if (worker != m_entities.workers.end())
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
    return m_entities.headquarters.size();
}

const Headquarters* Faction::getMainHeadquarters() const
{
    return !m_entities.headquarters.empty() ? &m_entities.headquarters.front() : nullptr;
}

const Headquarters* Faction::getClosestHeadquarters(const glm::vec3& position) const
{
	const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_entities.headquarters)
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

const std::vector<Headquarters>& Faction::GetHeadquarters() const
{
    return m_entities.headquarters;
}

const std::vector<ConstSafePTR<Entity>>& Faction::getEntities() const
{
    return m_entities.all;
}

const Entity* Faction::getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits) const
{
    const Entity* closestEntity = nullptr;
    if (prioritizeUnits)
    {
        float closestEntityDistance = maxDistance * maxDistance;
        for (const auto& entity : m_entities.all)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (!closestEntity && distance < closestEntityDistance)
            {
                closestEntity = *entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && Globals::BUILDING_TYPES.isMatch(closestEntity->getEntityType()) &&
                Globals::UNIT_TYPES.isMatch(entity->getEntityType()) &&
                Globals::getSqrDistance(entity->getPosition(), position) < maxDistance * maxDistance)
            {
                closestEntity = *entity;
                closestEntityDistance = distance;
            }
            else if (closestEntity && distance < closestEntityDistance)
            {
                closestEntity = *entity;
                closestEntityDistance = distance;
            }
        }
    }
    else
    {
        float closestEntityDistance = maxDistance * maxDistance;
        for (const auto& entity : m_entities.all)
        {
            float distance = Globals::getSqrDistance(entity->getPosition(), position);
            if (distance < closestEntityDistance)
            {
                closestEntity = *entity;
                closestEntityDistance = distance;
            }
        }
    }

    return closestEntity;
}

const Entity* Faction::getEntity(const AABB& aabb, int entityID) const
{
    const auto entity = std::find_if(m_entities.all.cbegin(), m_entities.all.cend(), [&aabb, entityID](auto& entity)
    {
        return entity->getID() == entityID && entity->getAABB().contains(aabb);
    });

    if (entity != m_entities.all.cend())
    {
        return *(*entity);
    }
    return nullptr;
}

const Entity* Faction::getEntity(const glm::vec3& position) const
{
    const auto entity = std::find_if(m_entities.all.cbegin(), m_entities.all.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (entity != m_entities.all.cend())
    {
        return *(*entity);
    }
    return nullptr;
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
    switch (gameEvent.type)
    {
    case eGameEventType::TakeDamage:
    {
        assert(gameEvent.data.takeDamage.senderFaction != m_controller);
        const auto entity = std::find_if(m_entities.all.begin(), m_entities.all.end(), [id = gameEvent.data.takeDamage.targetID](const auto& entity)
        {
            return entity->getID() == id;
        });
        if (entity == m_entities.all.end())
        {
            break;
        }

        (*entity)->takeDamage(gameEvent.data.takeDamage, map);
        if (!(*entity)->isDead())
        {
            on_entity_taken_damage(gameEvent.data.takeDamage, **(*entity), map, factionHandler);
            break;
        }
        switch ((*entity)->getEntityType())
        {
        case eEntityType::Worker:
            removeEntity<Worker>(m_entities.workers, entity);
            break;
        case eEntityType::Unit:
            removeEntity<Unit>(m_entities.units, entity);
            break;
        case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot>(m_entities.supply_depots, entity);
            break;
        case eEntityType::Barracks:
            removeEntity<Barracks>(m_entities.barracks, entity);
            break;
        case eEntityType::Headquarters:
            removeEntity<Headquarters>(m_entities.headquarters, entity);
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_entities.turrets, entity);
            break;
        case eEntityType::Laboratory:
            removeEntity<Laboratory>(m_entities.laboratories, entity);
            break;
        default:
            assert(false);
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        for (auto& unit : m_entities.units)
        {
            unit.revalidate_movement_path(map);
        }

        for (auto& worker : m_entities.workers)
        {
            worker.revalidate_movement_path(map);
        }
        break;
    case eGameEventType::RepairEntity:
    {
        int entityID = gameEvent.data.repairEntity.entityID;
        auto entity = std::find_if(m_entities.all.begin(), m_entities.all.end(), [entityID](const auto& entity)
        {
            return entity->getID() == entityID;
        });
        if (entity != m_entities.all.end())
        {
            (*entity)->repair();
        }
    }
        break;
    case eGameEventType::IncreaseFactionShield:
        if (m_currentShieldAmount < Globals::MAX_FACTION_SHIELD_AMOUNT &&
            isAffordable(Globals::FACTION_SHIELD_INCREASE_COST) &&
            !m_entities.laboratories.empty())
        {
            for (auto& laboratory : m_entities.laboratories)
            {
                laboratory.handleEvent(gameEvent.data.increaseFactionShield);
            }
        }
        break;
    case eGameEventType::ForceSelfDestructEntity:
    {
        const auto entity = std::find_if(m_entities.all.begin(), m_entities.all.end(), [id = gameEvent.data.forceSelfDestructEntity.entityID](auto& entity)
        {
            return entity->getID() == id;
        });
        if (entity == m_entities.all.end())
        {
            break;
        }

        switch (gameEvent.data.forceSelfDestructEntity.entityType)
        {
        case eEntityType::Worker:
            removeEntity<Worker>(m_entities.workers, entity);
            break;
        case eEntityType::Unit:
            removeEntity<Unit>(m_entities.units, entity);
            break;
        case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot>(m_entities.supply_depots, entity);
            break;
        case eEntityType::Barracks:
            removeEntity<Barracks>(m_entities.barracks, entity);
            break;
        case eEntityType::Headquarters:
            removeEntity<Headquarters>(m_entities.headquarters, entity);
            break;
        case eEntityType::Turret:
            removeEntity<Turret>(m_entities.turrets, entity);
            break;
        case eEntityType::Laboratory:
            removeEntity<Laboratory>(m_entities.laboratories, entity);
            break;
        default:
            assert(false);
        }
        break;
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

    for (auto& worker : m_entities.workers)
    {
        if (worker.getCurrentState() == eWorkerState::Idle)
        {
            if (map.isCollidable(worker.getPosition()))
            {
                glm::vec3 destination(0.f);
                if (PathFinding::getInstance().getClosestAvailablePosition(worker, m_entities.workers, map, destination))
                {
                    worker.MoveTo(destination, map, false);
                }
            }
            else
            {
                for (const auto& otherWorker : m_entities.workers)
                {
                    if (worker.getID() != otherWorker.getID() &&
                        foundWorker(otherWorker.getID()) &&
                        otherWorker.getCurrentState() == eWorkerState::Idle &&
                        worker.getAABB().contains(otherWorker.getAABB()))
                    {
                        glm::vec3 destination(0.f);
                        if (PathFinding::getInstance().getClosestAvailablePosition(worker, m_entities.workers, map, destination))
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

void Faction::on_entity_creation(Entity& entity, const std::optional<int> spawner_id)
{
    m_currentResourceAmount -= Globals::ENTITY_RESOURCE_COSTS[static_cast<int>(entity.getEntityType())];
    m_currentPopulationAmount += Globals::ENTITY_POPULATION_COSTS[static_cast<int>(entity.getEntityType())];
    if (entity.getEntityType() == eEntityType::SupplyDepot)
    {
        m_currentPopulationLimit += Globals::POPULATION_INCREMENT;
    }

    m_entities.all.push_back(&entity);
}

const Headquarters* Faction::get_closest_headquarters(const glm::vec3& position) const
{
    const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_entities.headquarters)
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
    auto entity = std::find_if(m_entities.all.cbegin(), m_entities.all.cend(), [id](const auto& entity)
    {
        return entity->getID() == id;
    });
    if (entity != m_entities.all.cend())
    {
        return *(*entity);
    }
    return nullptr;
}

Barracks* Faction::CreateBarracks(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateEntity(m_entities.barracks, scheduled_building.entityType, std::nullopt, scheduled_building.position, *this);
}

Turret* Faction::CreateTurret(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateEntity(m_entities.turrets, scheduled_building.entityType, std::nullopt, scheduled_building.position, *this);
}

Headquarters* Faction::CreateHeadquarters(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateEntity(m_entities.headquarters, scheduled_building.entityType, std::nullopt, scheduled_building.position, *this);
}

Laboratory* Faction::CreateLaboratory(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateEntity(m_entities.laboratories, scheduled_building.entityType, std::nullopt, scheduled_building.position, *this);
}

SupplyDepot* Faction::CreateSupplyDepot(const WorkerScheduledBuilding& scheduled_building)
{
    return CreateEntity(m_entities.supply_depots, scheduled_building.entityType, std::nullopt, scheduled_building.position, *this);
}

void Faction::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
    for (auto& unit : m_entities.units)
    {
        unit.update(deltaTime, factionHandler, map);
    }

    for (auto& worker : m_entities.workers)
    {
        worker.update(deltaTime, map, factionHandler);
    }

    for (auto& barracks : m_entities.barracks)
    {
        barracks.update(deltaTime, *this, map);
    }

    for (auto& turret : m_entities.turrets)
    {
        turret.update(deltaTime, factionHandler, map);
    }

    for (auto& supplyDepot : m_entities.supply_depots)
    {
        supplyDepot.update(deltaTime);
    }

    for (auto& headquarters : m_entities.headquarters)
    {
        headquarters.update(deltaTime, *this, map);
    }

    for (auto& laboratory : m_entities.laboratories)
    {
        laboratory.update(deltaTime);
    }
}

void Faction::delayed_update(const Map& map, FactionHandler& factionHandler)
{
    for (auto& unit : m_entities.units)
    {
        unit.delayed_update(factionHandler, map);
    }

    for (auto& worker : m_entities.workers)
    {
        worker.delayed_update(map, factionHandler);
    }

    handleWorkerCollisions(map);
}

void Faction::render(ShaderHandler& shaderHandler) const
{
    for (const auto& unit : m_entities.units)
    {
        unit.render(shaderHandler, m_controller);
    }

    for (const auto& worker : m_entities.workers)
    {
        worker.render(shaderHandler, m_controller);
    }

    for (const auto& supplyDepot : m_entities.supply_depots)
    {
        supplyDepot.render(shaderHandler, m_controller);
    }

    for (const auto& barracks : m_entities.barracks)
    {
        barracks.render(shaderHandler, m_controller);
    }

    for (const auto& turret : m_entities.turrets)
    {
        turret.render(shaderHandler, m_controller);
    }

    for (const auto& headquarters : m_entities.headquarters)
    {
        headquarters.render(shaderHandler, m_controller);
    }
 
    for (const auto& laboratory : m_entities.laboratories)
    {
        laboratory.render(shaderHandler, m_controller);
    }
}

void Faction::renderPlannedBuildings(ShaderHandler& shaderHandler) const
{
    for (const auto& worker : m_entities.workers)
    {
        worker.renderBuildingCommands(shaderHandler);
    }
}

void Faction::renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
    for (const auto& entity : m_entities.all)
    {
        entity->render_status_bars(shaderHandler, camera, windowSize);
    }
}

#ifdef RENDER_PATHING
void Faction::renderPathing(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_entities.units)
    {
        unit.render_path(shaderHandler);
    }

    for (auto& worker : m_entities.workers)
    {
        worker.render_path(shaderHandler);
    }
}
#endif // RENDER_PATHING

#ifdef RENDER_AABB
void Faction::renderAABB(ShaderHandler& shaderHandler)
{
    for (auto& unit : m_entities.units)
    {
        unit.renderAABB(shaderHandler);
    }

    for (auto& worker : m_entities.workers)
    {
        worker.renderAABB(shaderHandler);
    }

    for (auto& supplyDepot : m_entities.supply_depots)
    {
        supplyDepot.renderAABB(shaderHandler);
    }

    for (auto& barracks : m_entities.barracks)
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
    for (const auto& worker : m_entities.workers)
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
    return std::find_if(m_entities.workers.cbegin(), m_entities.workers.cend(), [entityType](auto& worker)
    {
        return worker.isInBuildQueue(entityType);
    }) != m_entities.workers.cend();
}

bool Faction::IsEntityCreatable(const eEntityType type) const
{
    const size_t entity_count = std::count_if(m_entities.all.cbegin(), m_entities.all.cend(), [type](const auto& entity)
    {
        return entity->getEntityType() == type;
    });

    return
        entity_count < MAX_ENTITY_QUANTITIES[static_cast<int>(type)] 
        && isAffordable(type) 
        && !isExceedPopulationLimit(type);
}

bool Faction::increaseShield(const Laboratory& laboratory)
{
    assert(laboratory.getShieldUpgradeCounter() > 0);
    if (isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
    {
        ++m_currentShieldAmount;
        m_currentResourceAmount -= Globals::FACTION_SHIELD_INCREASE_COST;

        for (auto& entity : m_entities.all)
        {
            entity->increaseMaximumShield(*this);
        }

        return true;
    }

    return false;
}

bool Faction::is_laboratory_built() const
{
    return !m_entities.laboratories.empty();
}

bool Faction::isMineralInUse(const Mineral& mineral) const
{
    return std::any_of(m_entities.workers.cbegin(), m_entities.workers.cend(), [&mineral](auto& worker)
    {
        return worker.getMineralToHarvest() && worker.getMineralToHarvest()->getPosition() == mineral.getPosition();
    });
}

Entity* Faction::createUnit(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map)
{
    return CreateEntity(m_entities.units, entity_to_spawn.type, entity_to_spawn.spawner_id, *this, entity_to_spawn, map);
}

Entity* Faction::createWorker(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map)
{
    return CreateEntity(m_entities.workers, entity_to_spawn.type, entity_to_spawn.spawner_id, *this, entity_to_spawn, map);
}