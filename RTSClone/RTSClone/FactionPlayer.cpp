#include "FactionPlayer.h"
#include "Globals.h"
#include "glad.h"
#include "Camera.h"
#include "Map.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "Mineral.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "GameEvents.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"
#include "Level.h"
#include <assert.h>
#include <array>
#include <algorithm>

namespace
{
    constexpr float PLANNED_BUILDING_OPACITY = 0.3f;
    constexpr glm::vec3 VALID_PLANNED_BUILDING_COLOR{ 0.0f, 1.0f, 0.0f };
    constexpr glm::vec3 INVALID_PLANNED_BUILDING_COLOR{ 1.0f, 0.0f, 0.0f };

    void moveSelectedEntitiesToAttackPosition(std::vector<Entity*>& selectedEntities, const Entity& targetEntity, 
        const Faction& targetFaction, const Map& map, FactionHandler& factionHandler)
    {
        assert(!selectedEntities.empty());
        std::sort(selectedEntities.begin(), selectedEntities.end(), [&](const auto& selectedUnitA, const auto& selectedUnitB)
        {
            return Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitA->getPosition()) <
                Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitB->getPosition());
        });

        for (const auto& selectedEntity : selectedEntities)
        {
            switch (selectedEntity->getEntityType())
            {
            case eEntityType::Unit:
                static_cast<Unit&>(*selectedEntity).moveToAttackPosition(targetEntity, targetFaction, map, factionHandler);
                break;
            case eEntityType::Worker:
                break;
            default:
                assert(false);
            }
        }
    }

    void setSelectedEntities(std::vector<Entity*>& selectedUnits, std::vector<Unit*>& units, std::vector<Worker*>& workers)
    {
        selectedUnits.clear();

        for (auto& unit : units)
        {
            if (unit->isSelected())
            {
                selectedUnits.push_back(&*unit);
            }
        }

        for (auto& worker : workers)
        {
            if (worker->isSelected())
            {
                selectedUnits.push_back(&*worker);
            }
        }
    }

    glm::vec3 getAveragePosition(std::vector<Entity*>& selectedEntities)
    {
        assert(!selectedEntities.empty()); 
        std::sort(selectedEntities.begin(), selectedEntities.end(), [](const auto& unitA, const auto& unitB)
        {
            return glm::all(glm::lessThan(unitA->getPosition(), unitB->getPosition()));
        });

        glm::vec3 total(0.0f, 0.0f, 0.0f);
        for (const auto& selectedUnit : selectedEntities)
        {
            total += selectedUnit->getPosition();
        }
    
        return { total.x / selectedEntities.size(), total.y / selectedEntities.size(), total.z / selectedEntities.size() };
    }
}

//FactionPlayerPlannedBuilding
FactionPlayerPlannedBuilding::FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position)
    : m_model(ModelManager::getInstance().getModel(MODEL_NAMES[static_cast<int>(gameEvent.entityType)])),
    m_builderID(gameEvent.targetID),
    m_entityType(gameEvent.entityType),
    m_position(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position)))
{}

const glm::vec3& FactionPlayerPlannedBuilding::getPosition() const
{
    return m_position;
}

int FactionPlayerPlannedBuilding::getBuilderID() const
{
    return m_builderID;
}

eEntityType FactionPlayerPlannedBuilding::getEntityType() const
{
    return m_entityType;
}

void FactionPlayerPlannedBuilding::handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map)
{
    if(event.type == sf::Event::MouseMoved)
    {
        assert(m_builderID != Globals::INVALID_ENTITY_ID);
        glm::vec3 position = 
            Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(camera.getRayToGroundPlaneIntersection(window)));
        if (map.isWithinBounds(AABB(position, ModelManager::getInstance().getModel(m_entityType))))
        {
            m_position = position;
        }
    }

}

void FactionPlayerPlannedBuilding::render(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const
{
    glm::vec3 color = (isOnValidPosition(baseHandler, map) ? VALID_PLANNED_BUILDING_COLOR : INVALID_PLANNED_BUILDING_COLOR);
    m_model.get().render(shaderHandler, m_position, color, PLANNED_BUILDING_OPACITY);
}

bool FactionPlayerPlannedBuilding::isOnValidPosition(const BaseHandler& baseHandler, const Map& map) const
{
    AABB buildingAABB(m_position, ModelManager::getInstance().getModel(m_entityType));
    assert(Globals::isOnMiddlePosition(m_position) && map.isWithinBounds(buildingAABB));
    if (!map.isAABBOccupied(buildingAABB))
    {
        switch (m_entityType)
        {
        case eEntityType::Headquarters:
        {
            for (const auto& base : baseHandler.getBases())
            {
                if (base.getCenteredPosition() == m_position)
                {
                    return true;
                }
            }
            
            return false;
        }
        default:
            return true;
        }
    }

    return false;
}

//FactionPlayer
FactionPlayer::FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation)
    : Faction(eFactionController::Player, hqStartingPosition, startingResources, startingPopulation),
    m_plannedBuilding(),
    m_entitySelector(),
    m_previousPlaneIntersection(),
    m_attackMoveSelected(false),
    m_addToDestinationQueue(false),
    m_selectedEntities()
{}

const std::vector<Entity*>& FactionPlayer::getSelectedEntities() const
{
    return m_selectedEntities;
}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, 
    const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler, const MiniMap& miniMap, 
    const glm::vec3& levelSize)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            onLeftClick(window, camera, map, baseHandler);
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            onRightClick(window, camera, factionHandler, map, baseHandler, miniMap, levelSize);
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_attackMoveSelected = false;
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            if (m_entitySelector.isActive() && !m_selectedEntities.empty())
            {
                deselectEntities<Headquarters*>(m_headquarters);
                deselectEntities<Barracks*>(m_barracks);
                deselectEntities<Turret*>(m_turrets);
                deselectEntities<SupplyDepot*>(m_supplyDepots);
            }

            m_entitySelector.reset();
        }
        break;
    case sf::Event::MouseMoved:
        m_entitySelector.update(camera, window);
        if (m_plannedBuilding)
        {
            m_plannedBuilding->handleInput(currentSFMLEvent, camera, window, map);
        }
        break;
    case sf::Event::KeyPressed:
        switch (currentSFMLEvent.key.code)
        {
        case sf::Keyboard::A:
            m_attackMoveSelected = true;
            break;
        case sf::Keyboard::LShift:
            m_addToDestinationQueue = true;
            break;
        }
        break;
    case sf::Event::KeyReleased:
        m_addToDestinationQueue = false;
        break;
    }
}

void FactionPlayer::handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler)
{
    Faction::handleEvent(gameEvent, map, factionHandler);

    switch (gameEvent.type)
    {
    case eGameEventType::PlayerActivatePlannedBuilding:
        assert(m_selectedEntities.size() == 1 && 
            m_selectedEntities.front()->getEntityType() == eEntityType::Worker);

        m_plannedBuilding = 
            std::optional<FactionPlayerPlannedBuilding>(std::in_place, gameEvent.data.playerActivatePlannedBuilding, m_selectedEntities.front()->getPosition());
        break;
    case eGameEventType::PlayerSpawnEntity:
    {
        int targetEntityID = gameEvent.data.playerSpawnEntity.targetID;
        auto entity = std::find_if(m_entities.begin(), m_entities.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_entities.end())
        {
            switch ((*entity)->getEntityType())
            {
            case eEntityType::Barracks:
                static_cast<Barracks&>(*(*entity)).addUnitToSpawnQueue();
                break;
            case eEntityType::Headquarters:
                static_cast<Headquarters&>(*(*entity)).addWorkerToSpawnQueue();
                break;
            default:
                assert(false);
            }
        }
        break;
    }
    break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
    Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

    if (m_entitySelector.isActive())
    {
        selectEntities<Unit*>(m_units);
        selectEntities<Worker*>(m_workers);
        setSelectedEntities(m_selectedEntities, m_units, m_workers);

        if (m_selectedEntities.size() == 1)
        {
            GameEventHandler::getInstance().gameEvents.push(
                GameEvent::createSetTargetEntityGUI(getController(), m_selectedEntities.back()->getID(), m_selectedEntities.back()->getEntityType()));
        }
        else if (!m_selectedEntities.empty())
        {
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createResetTargetEntityGUI());
        }
    }

    broadcastToMessenger<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });
}

void FactionPlayer::render(ShaderHandler& shaderHandler) const
{
    Faction::render(shaderHandler);
}

void FactionPlayer::renderPlannedBuilding(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const
{
    if (m_plannedBuilding)
    {
        m_plannedBuilding->render(shaderHandler, baseHandler, map);
    }
}

void FactionPlayer::renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const
{
    m_entitySelector.render(window, shaderHandler);
}

void FactionPlayer::onEntityRemoval(const Entity& entity)
{
    int entityID = entity.getID();
    auto selectedUnit = std::find_if(m_selectedEntities.begin(), m_selectedEntities.end(), [entityID](const auto& selectedUnit)
    {
        return selectedUnit->getID() == entityID;
    });
    if (selectedUnit != m_selectedEntities.end())
    {
        m_selectedEntities.erase(selectedUnit);
    }

    if (m_plannedBuilding &&
        m_plannedBuilding->getBuilderID() == entityID)
    {
        m_plannedBuilding.reset();
    }
}

void FactionPlayer::instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters)
{
    for (auto& worker : m_workers)
    {
        if (worker->isSelected() && worker->isHoldingResources())
        {
            worker->moveTo(headquarters, map, eWorkerState::ReturningMineralsToHeadquarters);
        }
    }
}

int FactionPlayer::instructWorkerToBuild(const Map& map, const BaseHandler& baseHandler)
{
    int workerID = Globals::INVALID_ENTITY_ID;
    if (m_plannedBuilding)
    {
        workerID = m_plannedBuilding->getBuilderID();
        if (m_plannedBuilding->isOnValidPosition(baseHandler, map) &&
            isAffordable(m_plannedBuilding->getEntityType()))
        {
            auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [workerID](const auto& worker)
            {
                return worker->getID() == workerID;
            });
            assert(selectedWorker != m_workers.cend());
            if (selectedWorker != m_workers.end())
            {
				if (m_plannedBuilding->getEntityType() == eEntityType::Headquarters)
				{
                    if (const Base* base = baseHandler.getBase(m_plannedBuilding->getPosition()))
                    {
                        if ((*selectedWorker)->build(base->getCenteredPosition(), map, m_plannedBuilding->getEntityType(), base))
                        {
                            m_plannedBuilding.reset();
                        }
                    }
				}
                else
                {
					if ((*selectedWorker)->build(m_plannedBuilding->getPosition(), map, m_plannedBuilding->getEntityType()))
					{
						m_plannedBuilding.reset();
					}
                }
            }
        }
        else if (!isAffordable(m_plannedBuilding->getEntityType()))
        {
            m_plannedBuilding.reset();
        }
    }

    return workerID;
}

void FactionPlayer::moveSingularSelectedEntity(const glm::vec3& destination, const Map& map, Entity& selectedEntity, 
    FactionHandler& factionHandler, const BaseHandler& baseHandler) const
{
    switch (selectedEntity.getEntityType())
    {
    case eEntityType::Unit:
    {
        eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
        Unit& unit = static_cast<Unit&>(selectedEntity);
        if (m_addToDestinationQueue)
        {
            unit.add_destination(destination, map, factionHandler);
        }
        else
        {
            unit.moveTo(destination, map, factionHandler);
        }
    }
        break;
    case eEntityType::Worker:
    {
        Worker& selectedWorker = static_cast<Worker&>(selectedEntity);
        const Mineral* mineralToHarvest = baseHandler.getMineral(destination);
        if (mineralToHarvest)
        {
            bool mineralValid = false;
            if (!isMineralInUse(*mineralToHarvest))
            {
                mineralValid = true;
            }
            else
            {
                mineralToHarvest = baseHandler.getNearestAvailableMineralAtBase(*this, *mineralToHarvest, selectedWorker.getPosition());
                mineralValid = mineralToHarvest;
            }

            if (mineralValid)
            {
                selectedWorker.moveTo(*mineralToHarvest, map);
            }
        }
        else
        {
            auto selectedEntity = std::find_if(m_entities.cbegin(), m_entities.cend(), 
                [&destination, &selectedWorker](const auto& entity)
            {
                return entity->getAABB().contains(destination) && entity->getID() != selectedWorker.getID();
            });
            if (selectedEntity != m_entities.cend() &&
                (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
            {
                selectedWorker.repairEntity(*(*selectedEntity), map);
            }
            else
            {
                if (m_addToDestinationQueue)
                {
                    selectedWorker.add_destination(destination, map);
                }
                else
                { 
                    selectedWorker.moveTo(destination, map, eWorkerState::Moving);
                }
            }
        }
    }
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::moveMultipleSelectedEntities(const glm::vec3& destination, const Map& map, 
    FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    assert(!m_selectedEntities.empty());
    const Base* base = baseHandler.getBaseAtMineral(destination);
    if (base)
    {
        std::for_each(m_selectedEntities.begin(), m_selectedEntities.end(), [&](auto& selectedUnit)
        {
            if (selectedUnit->getEntityType() == eEntityType::Worker)
            {
                const Mineral* mineral = baseHandler.getNearestAvailableMineralAtBase(*this, *base, selectedUnit->getPosition());
                if (mineral)
                {
                    static_cast<Worker&>(*selectedUnit).moveTo(*mineral, map);
                }
            }
        });
    }
    else
    {
        auto selectedEntity = std::find_if(m_entities.cbegin(), m_entities.cend(), [&destination](const auto& entity)
        {
            return entity->getAABB().contains(destination);
        });

        if (selectedEntity != m_entities.cend())
        {
            for (auto& selectedWorker : m_selectedEntities)
            {
                if (selectedWorker->getEntityType() == eEntityType::Worker &&
                    (*selectedEntity)->getID() != selectedWorker->getID() &&
                    (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
                {
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedWorker->getPosition(),
                        (*selectedEntity)->getAABB(), map);

                    static_cast<Worker&>(*selectedWorker).repairEntity(*(*selectedEntity), map);
                }
            }
        }
        else
        {
            glm::vec3 averagePosition = getAveragePosition(m_selectedEntities);
            std::for_each(m_selectedEntities.begin(), m_selectedEntities.end(), [&](auto& selectedEntity)
            {
                switch (selectedEntity->getEntityType())
                {
                case eEntityType::Unit:
                {
                    eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
                    glm::vec3 position = destination - (averagePosition - selectedEntity->getPosition());
                    Unit& unit = static_cast<Unit&>(*selectedEntity);
                    if (m_addToDestinationQueue)
                    {
                        unit.add_destination(position, map, factionHandler);
                    }
                    else
                    {
                        unit.moveTo(position, map, factionHandler);
                    }
                }
                break;
                case eEntityType::Worker:
                {
                    glm::vec3 position = destination - (averagePosition - selectedEntity->getPosition());
                    Worker& worker = static_cast<Worker&>(*selectedEntity);
                    if (m_addToDestinationQueue)
                    {
                        worker.add_destination(position, map);
                    }
                    else
                    {
                        worker.moveTo(position, map);
                    }
                }
                break;
                default:
                    assert(false);
                }
            });
        }
    }
}

void FactionPlayer::onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, const BaseHandler& baseHandler)
{
    glm::vec3 planeIntersection = camera.getRayToGroundPlaneIntersection(window);
    m_entitySelector.setStartingPosition(window, planeIntersection);

    int workerIDSelected = instructWorkerToBuild(map, baseHandler);
    selectEntity<Unit>(m_units, planeIntersection, planeIntersection == m_previousPlaneIntersection);
    selectEntity<Worker>(m_workers, planeIntersection, planeIntersection == m_previousPlaneIntersection, workerIDSelected);
    setSelectedEntities(m_selectedEntities, m_units, m_workers);

    if (planeIntersection != m_previousPlaneIntersection)
    {
        m_previousPlaneIntersection = planeIntersection;
    }

    if (m_selectedEntities.empty())
    {
        selectEntity<Barracks>(m_barracks, planeIntersection);
        selectEntity<Turret>(m_turrets, planeIntersection);
        selectEntity<SupplyDepot>(m_supplyDepots, planeIntersection);
        selectEntity<Headquarters>(m_headquarters, planeIntersection);
        if (!m_laboratories.empty())
        {
            assert(m_laboratories.size() == 1);
            m_laboratories.front()->setSelected(m_laboratories.front()->getAABB().contains(planeIntersection));
        }
    }
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map,
    const BaseHandler& baseHandler, const MiniMap& minimap, const glm::vec3& levelSize)
{
    m_plannedBuilding.reset();
    glm::vec3 position(0.0f);
    if (minimap.isIntersecting(window))
    {
        glm::vec2 mousePosition = { sf::Mouse::getPosition(window).x, window.getSize().y - sf::Mouse::getPosition(window).y };
        position = { mousePosition.y / (minimap.getPosition().y + minimap.getSize().y) * levelSize.z, 
            Globals::GROUND_HEIGHT, mousePosition.x / (minimap.getPosition().x + minimap.getSize().x) * levelSize.x };
        position.x -= minimap.getPosition().x;
        position.z -= minimap.getPosition().y;
    }
    else
    {
        position = camera.getRayToGroundPlaneIntersection(window);
    }
    const Faction* targetFaction = nullptr;
    const Entity* targetEntity = nullptr;

    for (const auto& opposingFactions : factionHandler.getOpposingFactions(getController()))
    {
        targetEntity = opposingFactions.get().getEntity(position);
        if (targetEntity)
        {
            targetFaction = &opposingFactions.get();
            break;
        }
    }

    if (targetEntity)
    {
        assert(targetFaction);

        if (m_selectedEntities.size() == 1)
        {
            switch (m_selectedEntities.back()->getEntityType())
            {
            case eEntityType::Unit:
                static_cast<Unit&>(*m_selectedEntities.back()).
                    moveToAttackPosition(*targetEntity, *targetFaction, map, factionHandler);
                break;
            case eEntityType::Worker:
                break;
            default:
                assert(false);
            }
        }
        else if (!m_selectedEntities.empty())
        {
            moveSelectedEntitiesToAttackPosition(m_selectedEntities, *targetEntity, *targetFaction, map, factionHandler);
        }
    }
    else
    {
        for (auto& headquarters : m_headquarters)
        {
            if (headquarters->isSelected())
            {
                headquarters->setWaypointPosition(position, map);
            }
            else if (headquarters->getAABB().contains(position))
            {
                instructWorkerReturnMinerals(map, *headquarters);
            }
        }

        for (auto& barracks : m_barracks)
        {
            if (barracks->isSelected())
            {
                barracks->setWaypointPosition(position, map);
            }
        }

        if (m_selectedEntities.size() == 1)
        {
            moveSingularSelectedEntity(position, map, *m_selectedEntities.front(), factionHandler, baseHandler);
        }
        else if (!m_selectedEntities.empty())
        {
            moveMultipleSelectedEntities(position, map, factionHandler, baseHandler);
        }
    }
}