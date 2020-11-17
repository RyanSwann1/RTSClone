#include "FactionPlayer.h"
#include "Globals.h"
#include "glad.h"
#include "Camera.h"
#include "Map.h"
#include "ModelManager.h"
#include "PathFinding.h"
#include "PathFindingLocator.h"
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
    Unit* isSingularUnitSelected(std::list<Unit>& units, std::list<Worker>& workers)
    {
        Unit* selectedUnit = nullptr;

        for (auto& unit : units)
        {
            if (unit.isSelected())
            {
                if (selectedUnit)
                {
                    return nullptr;
                }

                selectedUnit = &unit;
            }
        }

        for (auto& worker : workers)
        {
            if (worker.isSelected())
            {
                if (selectedUnit)
                {
                    return nullptr;
                }

                selectedUnit = &worker;
            }
        }

        return selectedUnit;
    }

    void moveSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, const std::list<Unit>& units,
        const std::vector<Entity*>& entities, const std::vector<Mineral>& minerals, bool attackMoveSelected)
    {
        assert(selectedEntity.getEntityType() == eEntityType::Unit || selectedEntity.getEntityType() == eEntityType::Worker);
        if (selectedEntity.getEntityType() == eEntityType::Unit)
        {
            Unit& selectedUnit = static_cast<Unit&>(selectedEntity);
            selectedUnit.resetTarget();
            selectedUnit.moveTo(Globals::convertToNodePosition(mouseToGroundPosition), map,
                [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, units, selectedUnit); },
                (attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving));
        }
        else if(selectedEntity.getEntityType() == eEntityType::Worker)
        {
            Worker& selectedWorker = static_cast<Worker&>(selectedEntity);
            selectedWorker.resetTarget();

            auto mineralToHarvest = std::find_if(minerals.cbegin(), minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
            {
                return mineral.getAABB().contains(mouseToGroundPosition);
            });
            if (mineralToHarvest != minerals.cend())
            {
                selectedWorker.moveTo(PathFindingLocator::get().getClosestPositionOutsideAABB(selectedWorker.getPosition(),
                    mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map),
                    map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                    eUnitState::MovingToMinerals, &(*mineralToHarvest));

                return;
            }

            int selectedWorkerID = selectedWorker.getID();
            auto selectedEntity = std::find_if(entities.cbegin(), entities.cend(), [&mouseToGroundPosition, selectedWorkerID](const auto& entity)
            {
                return entity->getAABB().contains(mouseToGroundPosition) && entity->getID() != selectedWorkerID;
            });
            if (selectedEntity != entities.cend() &&
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

    void moveSelectedUnits(std::vector<Unit*>& selectedUnits, const glm::vec3& mouseToGroundPosition, const Map& map,
        const std::vector<Mineral>& minerals, bool attackMoveSelected, const std::list<Unit>& units,
        const std::vector<Entity*>& entities)
    {
        assert(!selectedUnits.empty());

        auto mineralToHarvest = std::find_if(minerals.cbegin(), minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
        {
            return mineral.getAABB().contains(mouseToGroundPosition);
        });

        if (mineralToHarvest != minerals.cend())
        {
            for (auto& selectedUnit : selectedUnits)
            {
                if (selectedUnit->getEntityType() == eEntityType::Worker)
                {
                    glm::vec3 destination = PathFindingLocator::get().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
                        mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map);

                    static_cast<Worker&>(*selectedUnit).moveTo(destination, map,
                        [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                        eUnitState::MovingToMinerals, &(*mineralToHarvest));
                }
            }

            return;
        }
        
        auto selectedEntity = std::find_if(entities.cbegin(), entities.cend(), [&mouseToGroundPosition](const auto& entity)
        {
            return entity->getAABB().contains(mouseToGroundPosition);
        });

        if (selectedEntity != entities.cend())
        {
            for (auto& selectedUnit : selectedUnits)
            {
                if (selectedUnit->getEntityType() == eEntityType::Worker && 
                    (*selectedEntity)->getID() != selectedUnit->getID() &&
                    (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
                {
                    glm::vec3 destination = PathFindingLocator::get().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
                        (*selectedEntity)->getAABB(), (*selectedEntity)->getPosition(), map);

                    static_cast<Worker&>(*selectedUnit).setBuildingToRepair(*(*selectedEntity), map);
                }
            }
        }
        else
        {
            std::sort(selectedUnits.begin(), selectedUnits.end(), [](const auto& unitA, const auto& unitB)
            {
                return glm::all(glm::lessThan(unitA->getPosition(), unitB->getPosition()));
            });

            glm::vec3 total(0.0f, 0.0f, 0.0f);
            for (const auto& selectedUnit : selectedUnits)
            {
                total += selectedUnit->getPosition();
            }

            glm::vec3 averagePosition = { total.x / selectedUnits.size(), total.y / selectedUnits.size(), total.z / selectedUnits.size() };

            for (auto& selectedUnit : selectedUnits)
            {
                switch (selectedUnit->getEntityType())
                {
                case eEntityType::Unit:
                {
                    selectedUnit->resetTarget();
                    eUnitState state = (attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);

                    selectedUnit->moveTo(Globals::convertToNodePosition(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition())), map,
                        [&](const glm::ivec2& position)
                    { return getAdjacentPositions(position, map, units, *selectedUnit, selectedUnits); }, state);
                }
                break;
                case eEntityType::Worker:
                    selectedUnit->resetTarget();
                    static_cast<Worker*>(selectedUnit)->moveTo(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition()), map,
                        [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
                    break;
                default:
                    assert(false);
                }
            }
        }
    }

    void moveSelectedUnitsToAttackPosition(std::vector<Unit*>& selectedUnits, const Entity& targetEntity, 
        eFactionController targetFaction, const Map& map)
    {
        assert(!selectedUnits.empty());
        
        std::sort(selectedUnits.begin(), selectedUnits.end(), [&](const auto& selectedUnitA, const auto& selectedUnitB)
        {
            return Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitA->getPosition()) <
                Globals::getSqrDistance(targetEntity.getPosition(), selectedUnitB->getPosition());
        });
        
        PathFindingLocator::get().clearAttackPositions();
        for (auto& selectedUnit : selectedUnits)
        {
            selectedUnit->moveToAttackPosition(targetEntity, targetFaction, map);
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
        m_plannedBuilding.set(gameEvent);
        break;
    case eGameEventType::PlayerSpawnUnit:
    {
        int targetEntityID = gameEvent.targetID;
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_allEntities.end())
        {
            addUnitToSpawn(gameEvent.entityType, map, static_cast<UnitSpawnerBuilding&>(*(*entity)));
        }
    }
    break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
    Faction::update(deltaTime, map, factionHandler);

    GameMessenger::getInstance().broadcast<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });
}

void FactionPlayer::updateSelectionBox()
{
    if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize())
    {
        selectUnits<Unit>(m_units, m_selectionBox);
        selectUnits<Worker>(m_workers, m_selectionBox);

        const Unit* selectedUnit = isSingularUnitSelected(m_units, m_workers);
        if (selectedUnit)
        {
            GameEventHandler::getInstance().gameEvents.push({ eGameEventType::SetTargetEntityGUI, getController(), selectedUnit->getID() });
        }
        else
        {
            GameEventHandler::getInstance().gameEvents.push({ eGameEventType::ResetTargetEntityGUI });
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

void FactionPlayer::instructWorkerReturnMinerals(const Map& map)
{
    for (auto& worker : m_workers)
    {
        if (worker.isSelected() && worker.isHoldingResources())
        {
            glm::vec3 destination = PathFindingLocator::get().getClosestPositionOutsideAABB(worker.getPosition(), m_HQ.getAABB(), m_HQ.getPosition(), map);
            worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eUnitState::ReturningMineralsToHQ);
        }
    }
}

void FactionPlayer::instructUnitToAttack(Unit& unit, const Entity& targetEntity, eFactionController targetEntityOwningFaction, const Map& map)
{
    unit.setTarget(targetEntityOwningFaction, targetEntity.getID());
    if (unit.getCurrentState() != eUnitState::AttackingTarget)
    {
        unit.moveTo(targetEntity.getPosition(), map,
            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, m_units, unit); });
    }
}

bool FactionPlayer::instructWorkerToBuild(const Map& map)
{
    if (map.isWithinBounds(m_plannedBuilding.getPosition()) && !map.isPositionOccupied(m_plannedBuilding.getPosition()) && 
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
    selectEntity<Barracks>(m_barracks, mouseToGroundPosition);
    selectEntity<Turret>(m_turrets, mouseToGroundPosition);
    selectEntity<SupplyDepot>(m_supplyDepots, mouseToGroundPosition);
    m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map)
{
    m_plannedBuilding.setActive(false);
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
    eFactionController targetEntityFaction;
    const Entity* targetEntity = nullptr;
    for (const auto& faction : factionHandler.getOpposingFactions(getController()))
    {
        targetEntity = faction->getEntity(mouseToGroundPosition);
        if (targetEntity)
        {
            targetEntityFaction = faction->getController();
            break;
        }
    }
    if (targetEntity)
    {
        if (!isSingularUnitSelected(m_units, m_workers))
        {
            assignSelectedUnits();
            if (!m_selectedUnits.empty())
            {
                moveSelectedUnitsToAttackPosition(m_selectedUnits, *targetEntity, targetEntityFaction, map);
            }
        }
        else
        {
            for (auto& unit : m_units)
            {
                if (unit.isSelected())
                {
                    instructUnitToAttack(unit, *targetEntity, targetEntityFaction, map);
                }
            }
        }
    }
    else if (m_HQ.isSelected())
    {
        m_HQ.setWaypointPosition(mouseToGroundPosition, map);
    }
    else
    {
        for (auto& barracks : m_barracks)
        {
            if (barracks.isSelected())
            {
                barracks.setWaypointPosition(mouseToGroundPosition, map);
            }
        }

        Unit* selectedUnit = isSingularUnitSelected(m_units, m_workers);
        if (selectedUnit)
        {
            moveSelectedUnit(mouseToGroundPosition, map, *selectedUnit, m_units, m_allEntities, m_minerals, m_attackMoveSelected);
        }
        else
        {
            assignSelectedUnits();
            if (!m_selectedUnits.empty())
            {
                moveSelectedUnits(m_selectedUnits, mouseToGroundPosition, map, m_minerals, m_attackMoveSelected, m_units, m_allEntities);
            }
        }

        if (m_HQ.getAABB().contains(mouseToGroundPosition))
        {
            instructWorkerReturnMinerals(map);
        }
    }
}

void FactionPlayer::assignSelectedUnits()
{
    m_selectedUnits.clear();

    for (auto& unit : m_units)
    {
        if (unit.isSelected())
        {
            m_selectedUnits.push_back(&unit);
        }
    }

    for (auto& worker : m_workers)
    {
        if (worker.isSelected())
        {
            m_selectedUnits.push_back(&worker);
        }
    }
}