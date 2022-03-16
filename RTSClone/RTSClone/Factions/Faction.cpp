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
    m_currentPopulationLimit(startingPopulationCap),
    m_getClosestHeadquatersSubscriber(getController(), [this](const GameMessages::GetClosestHeadquarters& message) { return get_closest_headquarters(message); }),
    m_getEntitySubscriber(getController(), [this](const GameMessages::GetEntity& message) { return get_entity(message); }),
    m_createBuildingSubscriber(getController(), [this](const GameMessages::CreateBuilding& message) { return create_building(message); })
{
    m_entities.reserve(MAX_ENTITIES);
    m_units.reserve(Globals::MAX_UNITS);
    m_workers.reserve(Globals::MAX_WORKERS);
    m_headquarters.reserve(Globals::MAX_HEADQUARTERS);
    m_supplyDepots.reserve(Globals::MAX_SUPPLY_DEPOTS);
    m_barracks.reserve(Globals::MAX_BARRACKS);
    m_turrets.reserve(Globals::MAX_TURRETS);
    m_laboratories.reserve(Globals::MAX_LABORATORIES);

    std::unique_ptr<Entity>& headquarters = m_entities.emplace_back(std::make_unique<Headquarters>(hqStartingPosition, *this));
    m_headquarters.emplace_back(static_cast<Headquarters*>(headquarters.get()));
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

int Faction::getLaboratoryCount() const
{
    return static_cast<int>(m_laboratories.size());
}

const Headquarters* Faction::getMainHeadquarters() const
{
    return !m_headquarters.empty() ? &*m_headquarters.front() : nullptr;
}

const Headquarters* Faction::getClosestHeadquarters(const glm::vec3& position) const
{
	const Headquarters* closestHeadquarters = nullptr;
    std::for_each(m_headquarters.cbegin(), m_headquarters.cend(), 
        [&, distance = std::numeric_limits<float>::max()](const auto& headquarters) mutable
    {
        float result = Globals::getSqrDistance(headquarters->getPosition(), position);
        if (result < distance)
        {
            distance = result;
            closestHeadquarters = headquarters;
        }
    });

	return closestHeadquarters;
}

eFactionController Faction::getController() const
{
    return m_controller;
}

const std::vector<std::unique_ptr<Entity>>& Faction::getEntities() const
{
    return m_entities;
}

const Entity* Faction::getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits) const
{
    const Entity* closestEntity = nullptr;
    if (prioritizeUnits)
    {
        float closestEntityDistance = maxDistance * maxDistance;
        for (const auto& entity : m_entities)
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
        for (const auto& entity : m_entities)
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

const Entity* Faction::getEntity(const AABB& AABB, int entityID, eEntityType entityType) const
{
	const Entity* entity = nullptr;
	switch (entityType)
	{
	case eEntityType::Unit:
		entity = getEntity<std::vector<Unit*>>(m_units, entityID, AABB);
		break;
	case eEntityType::Worker:
		entity = getEntity<std::vector<Worker*>>(m_workers, entityID, AABB);
		break;
	case eEntityType::Headquarters:
		entity = getEntity<std::vector<Headquarters*>>(m_headquarters, entityID, AABB);
		break;
	case eEntityType::SupplyDepot:
		entity = getEntity<std::vector<SupplyDepot*>>(m_supplyDepots, entityID, AABB);
		break;
	case eEntityType::Barracks:
		entity = getEntity<std::vector<Barracks*>>(m_barracks, entityID, AABB);
		break;
	case eEntityType::Turret:
		entity = getEntity<std::vector<Turret*>>(m_turrets, entityID, AABB);
		break;
	case eEntityType::Laboratory:
		entity = getEntity<std::vector<Laboratory*>>(m_laboratories, entityID, AABB);
		break;
	default:
		assert(false);
	}

	return entity;
}

const Entity* Faction::getEntity(const glm::vec3& position) const
{
    const auto entity = std::find_if(m_entities.cbegin(), m_entities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    return entity != m_entities.cend() ? (*entity).get() : nullptr;
}

const Entity* Faction::getEntity(int entityID, eEntityType entityType) const
{
    const Entity* entity = nullptr; 
    switch(entityType)
    {
        case eEntityType::Unit:
			entity = getEntity<std::vector<Unit*>>(m_units, entityID);
			break;
        case eEntityType::Worker:
			entity = getEntity<std::vector<Worker*>>(m_workers, entityID);
			break;
        case eEntityType::Headquarters:
			entity = getEntity<std::vector<Headquarters*>>(m_headquarters, entityID);
			break;
        case eEntityType::SupplyDepot:
			entity = getEntity<std::vector<SupplyDepot*>>(m_supplyDepots, entityID);
			break;
        case eEntityType::Barracks:
			entity = getEntity<std::vector<Barracks*>>(m_barracks, entityID);
			break;
        case eEntityType::Turret:
			entity = getEntity<std::vector<Turret*>>(m_turrets, entityID);
			break;
        case eEntityType::Laboratory:
			entity = getEntity<std::vector<Laboratory*>>(m_laboratories, entityID);
			break;
        default:
            assert(false);
    }

    return entity;
}

void Faction::handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler)
{
    switch (gameEvent.type)
    {
    case eGameEventType::TakeDamage:
    {
        assert(gameEvent.data.takeDamage.senderFaction != m_controller);
        int targetID = gameEvent.data.takeDamage.targetID;
        auto entity = std::find_if(m_entities.begin(), m_entities.end(), [targetID](const auto& entity)
        {
            return entity->getID() == targetID;
        });
        if (entity != m_entities.end())
        {
            (*entity)->takeDamage(gameEvent.data.takeDamage, map);
            if (!(*entity)->isDead())
            {
                on_entity_taken_damage(gameEvent.data.takeDamage, *(*entity), map, factionHandler);
                break;
            }
            switch ((*entity)->getEntityType())
            {
            case eEntityType::Worker:
                removeEntity<Worker*>(m_workers, targetID, entity);
                break;
            case eEntityType::Unit:
                removeEntity<Unit*>(m_units, targetID, entity);
                break;
            case eEntityType::SupplyDepot:
                removeEntity<SupplyDepot*>(m_supplyDepots, targetID, entity);
                break;
            case eEntityType::Barracks:
                removeEntity<Barracks*>(m_barracks, targetID, entity);
                break;
            case eEntityType::Headquarters:
                removeEntity<Headquarters*>(m_headquarters, targetID, entity);
                if (m_headquarters.empty())
                {
                    Level::add_event(GameEvent::create<EliminateFactionEvent>({}));
                }
                break;
            case eEntityType::Turret:
                removeEntity<Turret*>(m_turrets, targetID, entity);
                break;
            case eEntityType::Laboratory:
                removeEntity<Laboratory*>(m_laboratories, targetID, entity);
                break;
            default:
                assert(false);
            }
        }
    }
        break;
    case eGameEventType::RevalidateMovementPaths:
        revalidateExistingUnitPaths(map);
        break;
    case eGameEventType::RepairEntity:
    {
        int entityID = gameEvent.data.repairEntity.entityID;
        auto entity = std::find_if(m_entities.begin(), m_entities.end(), [entityID](const auto& entity)
        {
            return entity->getID() == entityID;
        });
        if (entity != m_entities.end())
        {
            (*entity)->repair();
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
            removeEntity<Worker*>(m_workers, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Unit:
            removeEntity<Unit*>(m_units, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::SupplyDepot:
            removeEntity<SupplyDepot*>(m_supplyDepots, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Barracks:
            removeEntity<Barracks*>(m_barracks, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Headquarters:
            removeEntity<Headquarters*>(m_headquarters, gameEvent.data.forceSelfDestructEntity.entityID);
			if (m_headquarters.empty())
			{
                Level::add_event(GameEvent::create<EliminateFactionEvent>({}));
			}
			break;
		case eEntityType::Turret:
            removeEntity<Turret*>(m_turrets, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		case eEntityType::Laboratory:
            removeEntity<Laboratory*>(m_laboratories, gameEvent.data.forceSelfDestructEntity.entityID);
			break;
		default:
			assert(false);
        }
        break;
    case eGameEventType::EntityIdle:
    {
        const int entityID = gameEvent.data.entityIdle.entityID;
        auto entity = std::find_if(m_entities.begin(), m_entities.end(), [entityID](const auto& entity)
        {
            return entity->getID() == entityID;
        });
        if (entity != m_entities.cend())
        {
            on_entity_idle(*(*entity), map, factionHandler);
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

    std::for_each(m_workers.begin(), m_workers.end(), [this, &map, foundWorker](auto& worker)
    {
		if (worker->getCurrentState() == eWorkerState::Idle)
		{
			if (map.isCollidable(worker->getPosition()))
			{
				glm::vec3 destination(0.f);
				if (PathFinding::getInstance().getClosestAvailablePosition(*worker, m_workers, map, destination))
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
						if (PathFinding::getInstance().getClosestAvailablePosition(*worker, m_workers, map, destination))
						{
							worker->moveTo(destination, map);
							break;
						}
					}
				}
			}
		}

		handledWorkers.push_back(*worker);
    });

    handledWorkers.clear();
}

const Headquarters* Faction::get_closest_headquarters(const GameMessages::GetClosestHeadquarters& message) const
{
    const Headquarters* closestHeadquarters = nullptr;
    float distance = std::numeric_limits<float>::max();
    for (const auto& headquarters : m_headquarters)
    {
        const float result = Globals::getSqrDistance(headquarters->getPosition(), message.position);
        if (result < distance)
        {
            distance = result;
            closestHeadquarters = headquarters;
        }
    }

    return closestHeadquarters;
}

const Entity* Faction::get_entity(const GameMessages::GetEntity& message) const
{
    auto entity = std::find_if(m_entities.cbegin(), m_entities.cend(), [message](const auto& entity)
    {
        return entity->getID() == message.entityID;
    });
    if (entity != m_entities.cend())
    {
        return &*(*entity);
    }
    return nullptr;
}

Entity* Faction::create_building(const GameMessages::CreateBuilding& message)
{
    assert(message.worker.getCurrentState() == eWorkerState::Building && !message.worker.get_scheduled_buildings().empty());

    eEntityType entityType = message.worker.get_scheduled_buildings().front().entityType;
    const glm::vec3& position = message.worker.get_scheduled_buildings().front().position;
    if (isAffordable(entityType) && !message.map.isPositionOccupied(position))
    {
        Entity* addedBuilding = nullptr;
        switch (entityType)
        {
        case eEntityType::SupplyDepot:
            if (m_supplyDepots.size() < Globals::MAX_SUPPLY_DEPOTS)
            {
                addedBuilding = createEntity<SupplyDepot>(m_supplyDepots, position);
                increasePopulationLimit();
            }
            break;
        case eEntityType::Barracks:
            if (m_barracks.size() < Globals::MAX_BARRACKS)
            {
                addedBuilding = createEntity<Barracks>(m_barracks, position);
            }
            break;
        case eEntityType::Turret:
            if (m_turrets.size() < Globals::MAX_TURRETS)
            {
                addedBuilding = createEntity<Turret>(m_turrets, position);
            }
            break;
        case eEntityType::Headquarters:
            if (m_headquarters.size() < Globals::MAX_HEADQUARTERS)
            {
                addedBuilding = createEntity<Headquarters>(m_headquarters, position);
            }
            break;
        case eEntityType::Laboratory:
            if (m_laboratories.size() < Globals::MAX_LABORATORIES)
            {
                addedBuilding = createEntity<Laboratory>(m_laboratories, position);
            }
            break;
        default:
            assert(false);
        }

        if (addedBuilding)
        {
            reduceResources(entityType);
            Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));

            return addedBuilding;
        }
    }

    return nullptr;
}

void Faction::update(float deltaTime, const Map& map, const FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
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
    for (const auto& entity : m_entities)
    {
        entity->renderHealthBar(shaderHandler, camera, windowSize);
        entity->renderShieldBar(shaderHandler, camera, windowSize);

        switch (entity->getEntityType())
        {
        case eEntityType::Barracks:
            static_cast<Barracks&>(*entity).renderProgressBar(shaderHandler, camera, windowSize);
            break;
        case eEntityType::Headquarters:
            static_cast<Headquarters&>(*entity).renderProgressBar(shaderHandler, camera, windowSize);
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
        unit->render_path(shaderHandler);
    }

    for (auto& worker : m_workers)
    {
        worker->render_path(shaderHandler);
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
        auto buildingCommand = std::find_if(worker->get_scheduled_buildings().cbegin(), worker->get_scheduled_buildings().cend(),
            [&AABB](const auto& buildingCommand)
        {
            return AABB.contains(buildingCommand.position);
        });
        if (buildingCommand != worker->get_scheduled_buildings().cend())
        {
            return true;
        }
    }

    return false;
}

bool Faction::isBuildingInAllWorkersQueue(eEntityType entityType) const
{
    return std::find_if(m_workers.cbegin(), m_workers.cend(), [entityType](const auto& worker)
    {
        return worker->isInBuildQueue(entityType);
    }) != m_workers.cend();
}

bool Faction::increaseShield(const Laboratory& laboratory)
{
    assert(laboratory.getShieldUpgradeCounter() > 0);
    if (isAffordable(Globals::FACTION_SHIELD_INCREASE_COST))
    {
        ++m_currentShieldAmount;
        m_currentResourceAmount -= Globals::FACTION_SHIELD_INCREASE_COST;

        for (auto& entity : m_entities)
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
    std::for_each(m_units.begin(), m_units.end(), [&map](auto& unit)
    {
		if (!unit->getMovementPath().empty())
		{
			unit->moveTo(unit->getMovementPath().front(), map, unit->getCurrentState());
		}
    });

    std::for_each(m_workers.begin(), m_workers.end(), [&map](auto& worker)
    {
		if (!worker->getMovementPath().empty())
		{
			switch (worker->getCurrentState())
			{
			case eWorkerState::Moving:
			case eWorkerState::ReturningMineralsToHeadquarters:
			case eWorkerState::MovingToBuildingPosition:
			case eWorkerState::MovingToRepairPosition:
				assert(!worker->getMovementPath().empty());
				worker->moveTo(worker->getMovementPath().front(), map, worker->getCurrentState());
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
    });
}

bool Faction::isMineralInUse(const Mineral& mineral) const
{
    return std::any_of(m_workers.cbegin(), m_workers.cend(), [&mineral](auto& worker)
    {
        return worker->getMineralToHarvest() && worker->getMineralToHarvest()->getPosition() == mineral.getPosition();
    });
}

Entity* Faction::createUnit(const Map& map, const Barracks& barracks, const FactionHandler& factionHandler)
{
    assert(barracks.getCurrentSpawnCount() > 0);
    glm::vec3 startingPosition(0.0f);
    if (m_units.size() < Globals::MAX_UNITS &&
        isAffordable(eEntityType::Unit) && 
        !isExceedPopulationLimit(eEntityType::Unit) &&
        PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(barracks, map, startingPosition))
    {
        glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, barracks.getPosition()), 0.0f };
        Entity* createdUnit = nullptr;
        if (barracks.isWaypointActive())
        {
            createdUnit = m_entities.emplace_back(std::make_unique<Unit>(
                *this, startingPosition, startingRotation, barracks.getWaypointPosition(), map)).get();
        }
        else
        {
            createdUnit = m_entities.emplace_back(std::make_unique<Unit>(*this, startingPosition, startingRotation, map)).get();
        }

        reduceResources(eEntityType::Unit);
        increaseCurrentPopulationAmount(eEntityType::Unit);
        m_units.push_back(static_cast<Unit*>(createdUnit));

        return &*m_units.back();
    }

    return nullptr;
}

Entity* Faction::createWorker(const Map& map, const Headquarters& headquarters)
{
    assert(headquarters.getCurrentSpawnCount() > 0);
    glm::vec3 startingPosition(0.0f);
    if (m_workers.size() < Globals::MAX_WORKERS &&
        isAffordable(eEntityType::Worker) && 
        !isExceedPopulationLimit(eEntityType::Worker) &&
        PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(headquarters, map, startingPosition))
    {
        glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, headquarters.getPosition()), 0.0f };
        Entity* createdWorker = nullptr;
        if (headquarters.isWaypointActive())
        {
            createdWorker = m_entities.emplace_back(std::make_unique<Worker>(
                *this, startingPosition, headquarters.getWaypointPosition(), map, startingRotation)).get();
        }
        else
        {
            createdWorker = m_entities.emplace_back(std::make_unique<Worker>(*this, map, startingPosition, startingRotation)).get();
        }

        reduceResources(eEntityType::Worker);
        increaseCurrentPopulationAmount(eEntityType::Worker);
        m_workers.push_back(static_cast<Worker*>(createdWorker));

        return &*m_workers.back();
    }

    return nullptr;
}