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
    const float PLANNED_BUILDING_OPACITY = 0.3f;
    const glm::vec3 VALID_PLANNED_BUILDING_COLOR{ 0.0f, 1.0f, 0.0f };
    const glm::vec3 INVALID_PLANNED_BUILDING_COLOR{ 1.0f, 0.0f, 0.0f };

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

    void setSelectedEntities(std::vector<Entity*>& selectedUnits, std::list<Unit>& units, std::list<Worker>& workers)
    {
        selectedUnits.clear();

        for (auto& unit : units)
        {
            if (unit.isSelected())
            {
                selectedUnits.push_back(&unit);
            }
        }

        for (auto& worker : workers)
        {
            if (worker.isSelected())
            {
                selectedUnits.push_back(&worker);
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
    m_position(Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position))),
    m_entityType(gameEvent.entityType)
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
        glm::vec3 position = camera.getRayToGroundPlaneIntersection(window);
        if (map.isWithinBounds(position))
        {
            m_position = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(position));
        }
    }
}

void FactionPlayerPlannedBuilding::render(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const
{
    glm::vec3 color = (isOnValidPosition(baseHandler, map) ? VALID_PLANNED_BUILDING_COLOR : INVALID_PLANNED_BUILDING_COLOR);
    m_model.render(shaderHandler, m_position, color, PLANNED_BUILDING_OPACITY);
}

bool FactionPlayerPlannedBuilding::isOnValidPosition(const BaseHandler& baseHandler, const Map& map) const
{
    assert(map.isWithinBounds(m_position) && Globals::isOnMiddlePosition(m_position));
    if (!map.isPositionOccupied(m_position))
    {
        switch (m_entityType)
        {
        case eEntityType::Headquarters:
        {
            for (const auto& base : baseHandler.getBases())
            {
                if (base.getConvertedPosition() == m_position)
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

//factionPlayer
FactionPlayer::FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation)
    : Faction(eFactionController::Player, hqStartingPosition, startingResources, startingPopulation),
    m_entitySelector(),
    m_previousPlaneIntersection(),
    m_attackMoveSelected(false)
{}

const std::vector<Entity*>& FactionPlayer::getSelectedEntities() const
{
    return m_selectedEntities;
}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, 
    const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler)
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
            onRightClick(window, camera, factionHandler, map, baseHandler);
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_attackMoveSelected = false;
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            if (m_entitySelector.isActive() && !m_selectedEntities.empty())
            {
                deselectEntities<Headquarters>(m_headquarters);
                deselectEntities<Barracks>(m_barracks);
                deselectEntities<Turret>(m_turrets);
                deselectEntities<SupplyDepot>(m_supplyDepots);
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
        }
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
            std::make_unique<FactionPlayerPlannedBuilding>(gameEvent.data.playerActivatePlannedBuilding, m_selectedEntities.front()->getPosition());
        break;
    case eGameEventType::PlayerSpawnEntity:
    {
        int targetEntityID = gameEvent.data.playerSpawnEntity.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_allEntities.end())
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
        selectEntities<Unit>(m_units);
        selectEntities<Worker>(m_workers);
        setSelectedEntities(m_selectedEntities, m_units, m_workers);

        if (m_selectedEntities.size() == 1)
        {
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createSetTargetEntityGUI(getController(), m_selectedEntities.back()->getID()));
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
        if (worker.isSelected() && worker.isHoldingResources())
        {
            worker.moveTo(headquarters, map, eWorkerState::ReturningMineralsToHeadquarters);
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
                return worker.getID() == workerID;
            });
            assert(selectedWorker != m_workers.cend());
            if (selectedWorker != m_workers.end() &&
                selectedWorker->build(m_plannedBuilding->getPosition(), map, m_plannedBuilding->getEntityType()))
            {
                m_plannedBuilding.reset();
            }
        }
        else if (!isAffordable(m_plannedBuilding->getEntityType()))
        {
            m_plannedBuilding.reset();
        }
    }

    return workerID;
}

void FactionPlayer::moveSingularSelectedEntity(const glm::vec3& planeIntersection, const Map& map, Entity& selectedEntity, 
    FactionHandler& factionHandler, const BaseHandler& baseHandler) const
{
    switch (selectedEntity.getEntityType())
    {
    case eEntityType::Unit:
    {
        static_cast<Unit&>(selectedEntity).resetTarget();
        static_cast<Unit&>(selectedEntity).moveTo(planeIntersection, map,
            factionHandler, (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving));
    }
        break;
    case eEntityType::Worker:
    {
        Worker& selectedWorker = static_cast<Worker&>(selectedEntity);
        const Mineral* mineralToHarvest = baseHandler.getMineral(planeIntersection);
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
            auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), 
                [&planeIntersection, &selectedWorker](const auto& entity)
            {
                return entity->getAABB().contains(planeIntersection) && entity->getID() != selectedWorker.getID();
            });
            if (selectedEntity != m_allEntities.cend() &&
                (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
            {
                selectedWorker.repairEntity(*(*selectedEntity), map);
            }
            else
            {
                selectedWorker.moveTo(planeIntersection, map);
            }
        }
    }
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::moveMultipleSelectedEntities(const glm::vec3& planeIntersection, const Map& map, 
    FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    assert(!m_selectedEntities.empty());
    const Base* base = baseHandler.getBaseAtMineral(planeIntersection);
    if (base)
    {
        for (auto& selectedUnit : m_selectedEntities)
        {
            if (selectedUnit->getEntityType() == eEntityType::Worker)
            {
                const Mineral* mineral = baseHandler.getNearestAvailableMineralAtBase(*this, *base, selectedUnit->getPosition());
                if (mineral)
                {
                    static_cast<Worker&>(*selectedUnit).moveTo(*mineral, map);
                }
            }
        }
    }
    else
    {
        auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&planeIntersection](const auto& entity)
        {
            return entity->getAABB().contains(planeIntersection);
        });

        if (selectedEntity != m_allEntities.cend())
        {
            for (auto& selectedUnit : m_selectedEntities)
            {
                if (selectedUnit->getEntityType() == eEntityType::Worker &&
                    (*selectedEntity)->getID() != selectedUnit->getID() &&
                    (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
                {
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedUnit->getPosition(),
                        (*selectedEntity)->getAABB(), map);

                    static_cast<Worker&>(*selectedUnit).repairEntity(*(*selectedEntity), map);
                }
            }
        }
        else
        {
            glm::vec3 averagePosition = getAveragePosition(m_selectedEntities);
            for (auto& selectedEntity : m_selectedEntities)
            {
                switch (selectedEntity->getEntityType())
                {
                case eEntityType::Unit:
                {
                    Unit& selectedUnit = static_cast<Unit&>(*selectedEntity);
                    selectedUnit.resetTarget();
                    eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
                    glm::vec3 destination = planeIntersection - (averagePosition - selectedEntity->getPosition());

                    selectedUnit.moveTo(destination, map, factionHandler, state);
                }
                break;
                case eEntityType::Worker:
                {
                    glm::vec3 destination = planeIntersection - (averagePosition - selectedEntity->getPosition());
                    static_cast<Worker*>(selectedEntity)->moveTo(destination, map);
                }
                break;
                default:
                    assert(false);
                }
            }
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
            m_laboratories.front().setSelected(m_laboratories.front().getAABB().contains(planeIntersection));
        }
    }
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map,
    const BaseHandler& baseHandler)
{
    m_plannedBuilding.reset();
    glm::vec3 planeIntersection = camera.getRayToGroundPlaneIntersection(window);
    const Faction* targetFaction = nullptr;
    const Entity* targetEntity = nullptr;

    for (const auto& opposingFactions : factionHandler.getOpposingFactions(getController()))
    {
        targetEntity = opposingFactions.get().getEntity(planeIntersection);
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
            if (headquarters.isSelected())
            {
                headquarters.setWaypointPosition(planeIntersection, map);
            }
            else if (headquarters.getAABB().contains(planeIntersection))
            {
                instructWorkerReturnMinerals(map, headquarters);
            }
        }

        for (auto& barracks : m_barracks)
        {
            if (barracks.isSelected())
            {
                barracks.setWaypointPosition(planeIntersection, map);
            }
        }

        if (m_selectedEntities.size() == 1)
        {
            moveSingularSelectedEntity(planeIntersection, map, *m_selectedEntities.front(), factionHandler, baseHandler);
        }
        else if (!m_selectedEntities.empty())
        {
            moveMultipleSelectedEntities(planeIntersection, map, factionHandler, baseHandler);
        }
    }
}