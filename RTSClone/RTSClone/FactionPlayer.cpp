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
#include "GameEvent.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"
#include <assert.h>
#include <array>
#include <algorithm>

namespace
{
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

    void setSelectedEntities(std::vector<Entity*>& selectedUnits, std::forward_list<Unit>& units, std::forward_list<Worker>& workers)
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

    void removeSelectEntity(std::vector<Entity*>& selectedEntities, int entityID)
    {
        auto selectedUnit = std::find_if(selectedEntities.begin(), selectedEntities.end(), [entityID](const auto& selectedUnit)
        {
            return selectedUnit->getID() == entityID;
        });
        if (selectedUnit != selectedEntities.end())
        {
            selectedEntities.erase(selectedUnit);
        }
    }
}

FactionPlayerPlannedBuilding::FactionPlayerPlannedBuilding()
    : m_active(false),
    m_workerID(Globals::INVALID_ENTITY_ID),
    m_position(),
    m_entityType()
{}

const glm::vec3& FactionPlayerPlannedBuilding::getPosition() const
{
    return m_position;
}

int FactionPlayerPlannedBuilding::getWorkerID() const
{
    return m_workerID;
}

eEntityType FactionPlayerPlannedBuilding::getEntityType() const
{
    return m_entityType;
}

bool FactionPlayerPlannedBuilding::isActive() const
{
    return m_active;
}

void FactionPlayerPlannedBuilding::deactivate()
{
    m_active = false;
    m_workerID = Globals::INVALID_ENTITY_ID;
}

void FactionPlayerPlannedBuilding::setPosition(const glm::vec3& newPosition, const Map& map)
{
    assert(m_workerID != Globals::INVALID_ENTITY_ID);

    if (map.isWithinBounds(newPosition) && !map.isPositionOccupied(newPosition))
    {
        m_position = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(newPosition));
    }
}

void FactionPlayerPlannedBuilding::activate(const PlayerActivatePlannedBuildingEvent& gameEvent)
{
    m_active = true;
    m_entityType = gameEvent.entityType;
    m_workerID = gameEvent.targetID;
}

void FactionPlayerPlannedBuilding::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
    if (m_active)
    {
        switch (m_entityType)
        {
        case eEntityType::SupplyDepot:
            ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Barracks:
            ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Turret:
            ModelManager::getInstance().getModel(TURRET_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Laboratory:
            ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        default:
            assert(false);
        }
    }
}

FactionPlayer::FactionPlayer(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    const std::vector<glm::vec3>& mineralPositions, int startingResources, int startingPopulationCap)
    : Faction(factionController, hqStartingPosition, mineralPositions, startingResources, startingPopulationCap),
    m_selectionBox(),
    m_previousMouseToGroundPosition(),
    m_attackMoveSelected(false)
{}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, 
    const Map& map, FactionHandler& factionHandler)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            onLeftClick(window, camera, map);
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            onRightClick(window, camera, factionHandler, map);
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_attackMoveSelected = false;
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize() && !m_selectedEntities.empty())
            {
                m_HQ.setSelected(false);
                deselectEntities<Barracks>(m_barracks);
                deselectEntities<Turret>(m_turrets);
                deselectEntities<SupplyDepot>(m_supplyDepots);
            }

            m_selectionBox.reset();
        }
        break;
    case sf::Event::MouseMoved:
        if (m_selectionBox.isActive())
        {
            m_selectionBox.setSize(camera.getMouseToGroundPosition(window));

        }
        else if (m_plannedBuilding.isActive())
        {
            m_plannedBuilding.setPosition(camera.getMouseToGroundPosition(window), map);
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
        m_plannedBuilding.activate(gameEvent.data.playerActivatePlannedBuilding);
        break;
    case eGameEventType::PlayerSpawnUnit:
    {
        int targetEntityID = gameEvent.data.playerSpawnUnit.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_allEntities.end())
        {
            addUnitToSpawn(gameEvent.data.playerSpawnUnit.entityType, map, static_cast<UnitSpawnerBuilding&>(*(*entity)), factionHandler);
        }
    }
    break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
    Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

    broadcastToMessenger<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });
}

void FactionPlayer::updateSelectionBox()
{
    if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize())
    {
        selectUnits<Unit>(m_units, m_selectionBox);
        selectUnits<Worker>(m_workers, m_selectionBox);
        setSelectedEntities(m_selectedEntities, m_units, m_workers);

        if (m_selectedEntities.size() == static_cast<size_t>(1))
        {
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createSetTargetEntityGUI(getController(), m_selectedEntities.back()->getID()));
        }
        else if(!m_selectedEntities.empty())
        {
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createResetTargetEntityGUI());
        }
    }
}

void FactionPlayer::render(ShaderHandler& shaderHandler) const
{
    Faction::render(shaderHandler);

    m_plannedBuilding.render(shaderHandler, getController());
}

void FactionPlayer::renderSelectionBox(const sf::Window& window) const
{
    m_selectionBox.render(window);
}

void FactionPlayer::onEntityRemoval(const Entity& entity)
{
    switch (entity.getEntityType())
    {
    case eEntityType::Worker:
        removeSelectEntity(m_selectedEntities, entity.getID());
        if (m_plannedBuilding.getWorkerID() == entity.getID())
        {
            m_plannedBuilding.deactivate();
        }
        break;
    case eEntityType::Unit:
        removeSelectEntity(m_selectedEntities, entity.getID());
        break;
    case eEntityType::HQ:
    case eEntityType::SupplyDepot:
    case eEntityType::Barracks:
    case eEntityType::Turret:
    case eEntityType::Laboratory:
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::instructWorkerReturnMinerals(const Map& map)
{
    for (auto& worker : m_workers)
    {
        if (worker.isSelected() && worker.isHoldingResources())
        {
            glm::vec3 destination = 
                PathFinding::getInstance().getClosestPositionToAABB(worker.getPosition(), m_HQ.getAABB(), map);
            worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eWorkerState::ReturningMineralsToHQ);
        }
    }
}

int FactionPlayer::instructWorkerToBuild(const Map& map)
{
    int workerID = Globals::INVALID_ENTITY_ID;
    if (m_plannedBuilding.isActive())
    {
        workerID = m_plannedBuilding.getWorkerID();
        if (map.isWithinBounds(m_plannedBuilding.getPosition()) &&
            !map.isPositionOccupied(m_plannedBuilding.getPosition()) &&
            isEntityAffordable(m_plannedBuilding.getEntityType()))
        {
            assert(m_plannedBuilding.getWorkerID() != Globals::INVALID_ENTITY_ID);
            auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [workerID](const auto& worker)
            {
                return worker.getID() == workerID;
            });

            assert(selectedWorker != m_workers.cend());
            if (selectedWorker != m_workers.end() &&
                Faction::instructWorkerToBuild(m_plannedBuilding.getEntityType(), m_plannedBuilding.getPosition(), map, *selectedWorker))
            {
                m_plannedBuilding.deactivate();
            }
        }
        else if (!isEntityAffordable(m_plannedBuilding.getEntityType()))
        {
            m_plannedBuilding.deactivate();
        }
    }

    return workerID;
}

void FactionPlayer::moveSingularSelectedEntity(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, 
    FactionHandler& factionHandler) const
{
    switch (selectedEntity.getEntityType())
    {
    case eEntityType::Unit:
    {
        Unit& selectedUnit = static_cast<Unit&>(selectedEntity);
        selectedUnit.resetTarget();
        selectedUnit.moveTo(Globals::convertToNodePosition(mouseToGroundPosition), map,
            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, factionHandler, selectedUnit); },
            factionHandler, (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving));
    }
        break;
    case eEntityType::Worker:
    {
        Worker& selectedWorker = static_cast<Worker&>(selectedEntity);

        auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
        {
            return mineral.getAABB().contains(mouseToGroundPosition);
        });
        if (mineralToHarvest != m_minerals.cend())
        {
            selectedWorker.moveTo(PathFinding::getInstance().getClosestPositionToAABB(selectedWorker.getPosition(),
                mineralToHarvest->getAABB(), map),
                map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eWorkerState::MovingToMinerals, &(*mineralToHarvest));
        }
        else
        {
            auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), 
                [&mouseToGroundPosition, &selectedWorker](const auto& entity)
            {
                return entity->getAABB().contains(mouseToGroundPosition) && entity->getID() != selectedWorker.getID();
            });
            if (selectedEntity != m_allEntities.cend() &&
                (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
            {
                selectedWorker.setEntityToRepair(*(*selectedEntity), map);
            }
            else
            {
                selectedWorker.moveTo(mouseToGroundPosition, map, [&](const glm::ivec2& position) 
                { return getAdjacentPositions(position, map); });
            }
        }
    }
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::moveMultipleSelectedEntities(const glm::vec3& mouseToGroundPosition, const Map& map, FactionHandler& factionHandler)
{
    assert(!m_selectedEntities.empty());

    auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
    {
        return mineral.getAABB().contains(mouseToGroundPosition);
    });

    if (mineralToHarvest != m_minerals.cend())
    {
        for (auto& selectedUnit : m_selectedEntities)
        {
            if (selectedUnit->getEntityType() == eEntityType::Worker)
            {
                glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedUnit->getPosition(),
                    mineralToHarvest->getAABB(), map);

                static_cast<Worker&>(*selectedUnit).moveTo(destination, map,
                    [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                    eWorkerState::MovingToMinerals, &(*mineralToHarvest));
            }
        }
    }
    else
    {
        auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&mouseToGroundPosition](const auto& entity)
        {
            return entity->getAABB().contains(mouseToGroundPosition);
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

                    static_cast<Worker&>(*selectedUnit).setEntityToRepair(*(*selectedEntity), map);
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
                    glm::vec3 destination = Globals::convertToNodePosition(mouseToGroundPosition - (averagePosition - selectedEntity->getPosition()));

                    selectedUnit.moveTo(destination, map, [&](const glm::ivec2& position)
                    { return getAdjacentPositions(position, map, factionHandler, selectedUnit); }, factionHandler, state);
                }
                break;
                case eEntityType::Worker:
                {
                    Worker& selectedWorker = static_cast<Worker&>(*selectedEntity);
                    glm::vec3 destination = mouseToGroundPosition - (averagePosition - selectedEntity->getPosition());
                    static_cast<Worker*>(selectedEntity)->moveTo(destination, map,
                        [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
                }
                break;
                default:
                    assert(false);
                }
            }
        }
    }
}

void FactionPlayer::onLeftClick(const sf::Window& window, const Camera& camera, const Map& map)
{
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);

    m_selectionBox.setStartingPosition(window, mouseToGroundPosition);
    
    int workerIDSelected = instructWorkerToBuild(map);
    selectEntity<Unit>(m_units, mouseToGroundPosition, mouseToGroundPosition == m_previousMouseToGroundPosition);
    selectEntity<Worker>(m_workers, mouseToGroundPosition, mouseToGroundPosition == m_previousMouseToGroundPosition, workerIDSelected);
    setSelectedEntities(m_selectedEntities, m_units, m_workers);

    if (mouseToGroundPosition != m_previousMouseToGroundPosition)
    {
        m_previousMouseToGroundPosition = mouseToGroundPosition;
    }

    if (m_selectedEntities.empty())
    {
        selectEntity<Barracks>(m_barracks, mouseToGroundPosition);
        selectEntity<Turret>(m_turrets, mouseToGroundPosition);
        selectEntity<SupplyDepot>(m_supplyDepots, mouseToGroundPosition);
        if (m_laboratory)
        {
            m_laboratory->setSelected(m_laboratory->getAABB().contains(mouseToGroundPosition));
        }
        m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
    }
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map)
{
    m_plannedBuilding.deactivate();
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
    const Faction* targetFaction = nullptr;
    const Entity* targetEntity = nullptr;

    for (const auto& opposingFactions : factionHandler.getOpposingFactions(getController()))
    {
        targetEntity = opposingFactions.get().getEntity(mouseToGroundPosition);
        if (targetEntity)
        {
            targetFaction = &opposingFactions.get();
            break;
        }
    }

    if (targetEntity)
    {
        assert(targetFaction);

        if (m_selectedEntities.size() == static_cast<size_t>(1))
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
        if (m_HQ.isSelected())
        {
            m_HQ.setWaypointPosition(mouseToGroundPosition, map);
        }
        else if (m_HQ.getAABB().contains(mouseToGroundPosition))
        {
            instructWorkerReturnMinerals(map);
        }

        for (auto& barracks : m_barracks)
        {
            if (barracks.isSelected())
            {
                barracks.setWaypointPosition(mouseToGroundPosition, map);
            }
        }

        if (m_selectedEntities.size() == static_cast<size_t>(1))
        {
            moveSingularSelectedEntity(mouseToGroundPosition, map, *m_selectedEntities.back(), factionHandler);
        }
        else if (!m_selectedEntities.empty())
        {
            moveMultipleSelectedEntities(mouseToGroundPosition, map, factionHandler);
        }
    }
}