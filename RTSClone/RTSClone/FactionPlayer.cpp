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
    void moveSelectedUnitsToAttackPosition(std::vector<Unit*>& selectedUnits, const Entity& targetEntity, 
        const Faction& targetFaction, const Map& map, const FactionPlayer& factionPlayer)
    {
        assert(!selectedUnits.empty());
        
        std::sort(selectedUnits.begin(), selectedUnits.end(), [&](const auto& selectedUnitA, const auto& selectedUnitB)
        {
            return Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitA->getPosition()) <
                Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitB->getPosition());
        });

        for (auto& selectedUnit : selectedUnits)
        {
            selectedUnit->moveToAttackPosition(targetEntity, targetFaction, map, factionPlayer);
        }
    }

    void setSelectedUnits(std::vector<Unit*>& selectedUnits, std::forward_list<Unit>& units, std::forward_list<Worker>& workers)
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

    glm::vec3 getAveragePosition(std::vector<Unit*>& selectedUnits)
    {
        assert(!selectedUnits.empty());
      
        std::sort(selectedUnits.begin(), selectedUnits.end(), [](const auto& unitA, const auto& unitB)
        {
            return glm::all(glm::lessThan(unitA->getPosition(), unitB->getPosition()));
        });

        glm::vec3 total(0.0f, 0.0f, 0.0f);
        for (const auto& selectedUnit : selectedUnits)
        {
            total += selectedUnit->getPosition();
        }
    
        return { total.x / selectedUnits.size(), total.y / selectedUnits.size(), total.z / selectedUnits.size() };
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
            if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize() && !m_selectedUnits.empty())
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
        m_plannedBuilding.handleEvent(gameEvent.data.playerActivatePlannedBuilding);
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
            addUnitToSpawn(gameEvent.data.playerSpawnUnit.entityType, map, static_cast<UnitSpawnerBuilding&>(*(*entity)));
        }
    }
    break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer)
{
    Faction::update(deltaTime, map, factionHandler, unitStateHandlerTimer);

    GameMessenger::getInstance().broadcast<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });
}

void FactionPlayer::updateSelectionBox()
{
    if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize())
    {
        selectUnits<Unit>(m_units, m_selectionBox);
        selectUnits<Worker>(m_workers, m_selectionBox);
        setSelectedUnits(m_selectedUnits, m_units, m_workers);

        if (m_selectedUnits.size() == static_cast<size_t>(1))
        {
            GameEventHandler::getInstance().gameEvents.push(GameEvent::createSetTargetEntityGUI(getController(), m_selectedUnits.back()->getID()));
        }
        else if(m_selectedUnits.size() > static_cast<size_t>(1))
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
    case eEntityType::Unit:
    {
        int entityID = entity.getID();
        auto selectedUnit = std::find_if(m_selectedUnits.begin(), m_selectedUnits.end(), [entityID](const auto& selectedUnit)
        {
            return selectedUnit->getID() == entityID;
        });
        if (selectedUnit != m_selectedUnits.end())
        {
            m_selectedUnits.erase(selectedUnit);
        }
    }
        break;
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
                eUnitState::ReturningMineralsToHQ);
        }
    }
}

bool FactionPlayer::instructWorkerToBuild(const Map& map)
{
    if (map.isWithinBounds(m_plannedBuilding.getPosition()) && 
        !map.isPositionOccupied(m_plannedBuilding.getPosition()) && 
        isEntityAffordable(m_plannedBuilding.getEntityType()))
    {
        int workerID = m_plannedBuilding.getWorkerID();
        assert(workerID != Globals::INVALID_ENTITY_ID);
        auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [workerID](const auto& worker)
        {
            if (workerID != Globals::INVALID_ENTITY_ID)
            {
                return worker.getID() == workerID && worker.isSelected();
            }
            else
            {
                return worker.isSelected();
            }
        });

        assert(selectedWorker != m_workers.cend());
        if (selectedWorker != m_workers.end())
        {
            return Faction::instructWorkerToBuild(m_plannedBuilding.getEntityType(), m_plannedBuilding.getPosition(), map, *selectedWorker);
        }
    }
    else if (!isEntityAffordable(m_plannedBuilding.getEntityType()))
    {
        m_plannedBuilding.setActive(false);
    }

    return false;
}

void FactionPlayer::moveSingularSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity) const
{
    switch (selectedEntity.getEntityType())
    {
    case eEntityType::Unit:
    {
        Unit& selectedUnit = static_cast<Unit&>(selectedEntity);
        selectedUnit.resetTarget();
        selectedUnit.moveTo(Globals::convertToNodePosition(mouseToGroundPosition), map,
            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
            (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving));
    }
        break;
    case eEntityType::Worker:
    {
        Worker& selectedWorker = static_cast<Worker&>(selectedEntity);
        selectedWorker.resetTarget();

        auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
        {
            return mineral.getAABB().contains(mouseToGroundPosition);
        });
        if (mineralToHarvest != m_minerals.cend())
        {
            selectedWorker.moveTo(PathFinding::getInstance().getClosestPositionToAABB(selectedWorker.getPosition(),
                mineralToHarvest->getAABB(), map),
                map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eUnitState::MovingToMinerals, &(*mineralToHarvest));
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
                selectedWorker.setBuildingToRepair(*(*selectedEntity), map);
            }
            else
            {
                selectedWorker.moveTo(mouseToGroundPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
            }
        }
    }
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::moveMultipleSelectedUnits(const glm::vec3& mouseToGroundPosition, const Map& map)
{
    assert(!m_selectedUnits.empty());

    auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
    {
        return mineral.getAABB().contains(mouseToGroundPosition);
    });

    if (mineralToHarvest != m_minerals.cend())
    {
        for (auto& selectedUnit : m_selectedUnits)
        {
            if (selectedUnit->getEntityType() == eEntityType::Worker)
            {
                glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedUnit->getPosition(),
                    mineralToHarvest->getAABB(), map);

                static_cast<Worker&>(*selectedUnit).moveTo(destination, map,
                    [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                    eUnitState::MovingToMinerals, &(*mineralToHarvest));
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
            for (auto& selectedUnit : m_selectedUnits)
            {
                if (selectedUnit->getEntityType() == eEntityType::Worker &&
                    (*selectedEntity)->getID() != selectedUnit->getID() &&
                    (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
                {
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedUnit->getPosition(),
                        (*selectedEntity)->getAABB(), map);

                    static_cast<Worker&>(*selectedUnit).setBuildingToRepair(*(*selectedEntity), map);
                }
            }
        }
        else
        {
            glm::vec3 averagePosition = getAveragePosition(m_selectedUnits);
            for (auto& selectedUnit : m_selectedUnits)
            {
                switch (selectedUnit->getEntityType())
                {
                case eEntityType::Unit:
                {
                    selectedUnit->resetTarget();
                    eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
                    glm::vec3 position = Globals::convertToNodePosition(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition()));

                    selectedUnit->moveTo(position, map, [&](const glm::ivec2& position)
                    { return getAdjacentPositions(position, map, m_units, *selectedUnit, m_selectedUnits); }, state);
                }
                break;
                case eEntityType::Worker:
                {
                    selectedUnit->resetTarget();
                    glm::vec3 position = mouseToGroundPosition - (averagePosition - selectedUnit->getPosition());

                    static_cast<Worker*>(selectedUnit)->moveTo(position, map,
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
    bool selectAllUnits = false;
    int entityIDSelected = Globals::INVALID_ENTITY_ID;
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
    if (mouseToGroundPosition != m_previousMouseToGroundPosition)
    {
        m_previousMouseToGroundPosition = mouseToGroundPosition;

        if (m_plannedBuilding.isActive() && instructWorkerToBuild(map))
        {
            entityIDSelected = m_plannedBuilding.getWorkerID();
            m_plannedBuilding.setActive(false);
        }
    }
    else
    {
        selectAllUnits = true;
    }
    
    m_selectionBox.setStartingPosition(window, mouseToGroundPosition);

    selectEntity<Unit>(m_units, mouseToGroundPosition, selectAllUnits);
    selectEntity<Worker>(m_workers, mouseToGroundPosition, selectAllUnits, entityIDSelected);
    setSelectedUnits(m_selectedUnits, m_units, m_workers);

    selectEntity<Barracks>(m_barracks, mouseToGroundPosition);
    selectEntity<Turret>(m_turrets, mouseToGroundPosition);
    selectEntity<SupplyDepot>(m_supplyDepots, mouseToGroundPosition);
    m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map)
{
    m_plannedBuilding.setActive(false);
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

        if (m_selectedUnits.size() == static_cast<size_t>(1))
        {
            m_selectedUnits.back()->moveToAttackPosition(*targetEntity, *targetFaction, map, factionHandler);
        }
        else if (m_selectedUnits.size() >= static_cast<size_t>(2))
        {
            moveSelectedUnitsToAttackPosition(m_selectedUnits, *targetEntity, *targetFaction, map, *this);
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

        if (m_selectedUnits.size() == static_cast<size_t>(1))
        {
            moveSingularSelectedUnit(mouseToGroundPosition, map, *m_selectedUnits.back());
        }
        else if (m_selectedUnits.size() >= static_cast<size_t>(2))
        {
            moveMultipleSelectedUnits(mouseToGroundPosition, map);
        }
    }
}