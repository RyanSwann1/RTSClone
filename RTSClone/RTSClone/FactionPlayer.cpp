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
#include <assert.h>
#include <array>
#include <algorithm>

FactionPlayer::FactionPlayer(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions)
    : Faction(factionController, hqStartingPosition, mineralPositions),
    m_selectionBox(),
    m_previousMouseToGroundPosition(),
    m_attackMoveSelected(false)
{}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map,
    const std::vector<const Faction*>& opposingFactions, EntityTarget& selectedTarget)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            bool selectAllUnits = false;
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            if (mouseToGroundPosition != m_previousMouseToGroundPosition)
            {
                m_previousMouseToGroundPosition = mouseToGroundPosition;
                selectAllUnits = false;
            
                if (m_plannedBuilding.active)
                {
                    m_plannedBuilding.active = false;
                    instructWorkerToBuild(m_plannedBuilding.entityType, mouseToGroundPosition,
                        map, m_plannedBuilding.workerID);
                }
            }
            else
            {
                selectAllUnits = true;
            }

            selectUnit<Unit>(m_units, mouseToGroundPosition, selectAllUnits);
            selectUnit<Worker>(m_workers, mouseToGroundPosition, selectAllUnits);

            m_selectionBox.setStartingPosition(window, mouseToGroundPosition);
            
            m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
            for (auto& barracks : m_barracks)
            {
                barracks.setSelected(barracks.getAABB().contains(mouseToGroundPosition));
            }
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            m_plannedBuilding.active = false;
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            eFactionController targetEntityOwningFaction;
            const Entity* targetEntity = nullptr;
            for (const auto& faction : opposingFactions)
            {
                targetEntity = faction->getEntity(mouseToGroundPosition);
                if (targetEntity)
                {
                    targetEntity;
                    targetEntityOwningFaction = faction->getController();
                    break;
                }
            }
            if (targetEntity)
            {
                for (auto& unit : m_units)
                {
                    if (unit.isSelected())
                    {
                        instructUnitToAttack(unit, *targetEntity, targetEntityOwningFaction, map);
                    }
                }
            }
            else if (m_HQ.isSelected())
            {
                m_HQ.setWaypointPosition(mouseToGroundPosition);
            }
            else if (m_HQ.getAABB().contains(mouseToGroundPosition))
            {
                instructWorkerReturnMinerals(map);
            }
            else
            {
                for (auto& barracks : m_barracks)
                {
                    if (barracks.isSelected())
                    {
                        barracks.setWaypointPosition(mouseToGroundPosition);
                    }
                }

                if (isOneUnitSelected())
                {
                    moveSingularSelectedUnit(mouseToGroundPosition, map);
                }
                else
                {
                    moveMultipleSelectedUnits(mouseToGroundPosition, map);
                }
            }
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
        if (m_selectionBox.active)
        {
            m_selectionBox.setSize(camera.getMouseToGroundPosition(window));
            const Entity* selectedEntity = nullptr;
            int entitiesSelected = 0;

            for (auto& unit : m_units)
            {
                unit.setSelected(m_selectionBox.AABB.contains(unit.getAABB()));
                if (unit.isSelected())
                {
                    ++entitiesSelected;
                    selectedEntity = &unit;
                }
            }

            for (auto& worker : m_workers)
            {
                worker.setSelected(m_selectionBox.AABB.contains(worker.getAABB()));
                if (worker.isSelected())
                {
                    ++entitiesSelected;
                    selectedEntity = &worker;
                }
            }

            if (entitiesSelected == 1)
            {
                assert(selectedEntity);
                selectedTarget.set(getController(), selectedEntity->getID());
            }
        }
        else if (m_plannedBuilding.active)
        {
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            if (Globals::isPositionInMapBounds(mouseToGroundPosition))
            {
                m_plannedBuilding.spawnPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(mouseToGroundPosition));
            }
        }
        break;
    case sf::Event::KeyPressed:
        switch (currentSFMLEvent.key.code)
        {
        case sf::Keyboard::U:
        {
            auto selectedBarracks = std::find_if(m_barracks.begin(), m_barracks.end(), [](const auto & barracks)
            {
                return barracks.isSelected();
            });
            if (selectedBarracks != m_barracks.end())
            {
                Faction::addUnitToSpawn(eEntityType::Unit, map, *selectedBarracks);
            }
        }
            break;
        case sf::Keyboard::W:
            if (m_HQ.isSelected())
            {
                Faction::addUnitToSpawn(eEntityType::Worker, map, m_HQ);
            }
            break;
        case sf::Keyboard::B:
            instructWorkerToBuild(eEntityType::SupplyDepot, camera.getMouseToGroundPosition(window), map);
            break;
        case sf::Keyboard::N:
            instructWorkerToBuild(eEntityType::Barracks, camera.getMouseToGroundPosition(window), map);
            break;
        case sf::Keyboard::A:
            m_attackMoveSelected = true;
            break;
        }
        break;
    }
}

void FactionPlayer::handleEvent(const GameEvent& gameEvent, const Map& map)
{
    Faction::handleEvent(gameEvent, map);

    switch (gameEvent.type)
    {
    case eGameEventType::ActivatePlayerPlannedBuilding:
        assert(gameEvent.entityType == eEntityType::Barracks || gameEvent.entityType == eEntityType::SupplyDepot);
        m_plannedBuilding.active = true;
        m_plannedBuilding.entityType = gameEvent.entityType;
        m_plannedBuilding.workerID = gameEvent.targetID;
        break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
    Faction::update(deltaTime, map, factionHandler);

    GameMessenger::getInstance().broadcast<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });
}

void FactionPlayer::render(ShaderHandler& shaderHandler) const
{
    Faction::render(shaderHandler);

    m_plannedBuilding.render(shaderHandler);
}

void FactionPlayer::renderSelectionBox(const sf::Window& window) const
{
    m_selectionBox.render(window);
}

bool FactionPlayer::isOneUnitSelected() const
{
    int unitSelectedCount = 0;

    for (const auto& unit : m_units)
    {
        if (unit.isSelected())
        {
            ++unitSelectedCount;
            if (unitSelectedCount > 1)
            {
                return false;
            }
        }
    }

    for (const auto& worker : m_workers)
    {
        if (worker.isSelected())
        {
            ++unitSelectedCount;
            if (unitSelectedCount > 1)
            {
                return false;
            }
        }
    }

    return unitSelectedCount == 1;
}

void FactionPlayer::moveSingularSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map)
{
    assert(isOneUnitSelected());

    auto selectedUnit = std::find_if(m_units.begin(), m_units.end(), [](const auto& unit) {
        return unit.isSelected() == true;
    });
    if (selectedUnit != m_units.end())
    {
        selectedUnit->resetTarget();
        eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
        selectedUnit->moveTo(Globals::convertToNodePosition(mouseToGroundPosition), map,
            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map, m_units, *selectedUnit); }, state);
    }
    else
    {
        auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [](const auto& worker) {
            return worker.isSelected() == true;
        });
        assert(selectedWorker != m_workers.end());
        selectedWorker->resetTarget();

        const Mineral* mineralToHarvest = nullptr;
        for (const auto& mineral : m_minerals)
        {
            if (mineral.getAABB().contains(mouseToGroundPosition))
            {
                mineralToHarvest = &mineral;
                break;
            }
        }

        if (mineralToHarvest)
        {
            glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(selectedWorker->getPosition(),
                mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map);
            
            selectedWorker->moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
                eUnitState::MovingToMinerals, mineralToHarvest);
        }
        else
        {
            selectedWorker->moveTo(mouseToGroundPosition, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
        }
    }
}

void FactionPlayer::moveMultipleSelectedUnits(const glm::vec3& mouseToGroundPosition, const Map& map)
{
    static std::vector<Unit*> selectedUnits;
 
    for (auto& unit : m_units)
    {
        if (unit.isSelected())
        {
            selectedUnits.push_back(&unit);
        }
    }

    for (auto& worker : m_workers)
    {
        if (worker.isSelected())
        {
            selectedUnits.push_back(&worker);
        }
    }   
    
    if (!selectedUnits.empty())
    {
        assert(!isOneUnitSelected());
        AABB selectionBoxAABB(selectedUnits);
        if (selectionBoxAABB.contains(mouseToGroundPosition))
        {
            std::vector<glm::vec3> unitFormationPositions = PathFinding::getInstance().getFormationPositions(mouseToGroundPosition,
                selectedUnits, map);

            if (unitFormationPositions.size() == selectedUnits.size())
            {
                for (int i = 0; i < unitFormationPositions.size(); ++i)
                {
                    selectedUnits[i]->moveTo(unitFormationPositions[i], map, 
                        [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
                }
            }
        }
        else
        {
            const Mineral* mineralToHarvest = nullptr;
            for (const auto& mineral : m_minerals)
            {
                if (mineral.getAABB().contains(mouseToGroundPosition))
                {
                    mineralToHarvest = &mineral;
                    break;
                }
            }

            if (mineralToHarvest)
            {
                for(auto& selectedUnit : selectedUnits)
                {
                    if (selectedUnit->getEntityType() == eEntityType::Worker)
                    {
                        glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
                            mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map);

                        static_cast<Worker*>(selectedUnit)->moveTo(destination, map, 
                            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
                            eUnitState::MovingToMinerals, mineralToHarvest);
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
                        eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);

                        selectedUnit->moveTo(Globals::convertToNodePosition(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition())), map,
                            [&](const glm::ivec2& position)
                        { return getAllAdjacentPositions(position, map, m_units, *selectedUnit, selectedUnits); }, state);
                    }
                        break;
                    case eEntityType::Worker:
                        selectedUnit->resetTarget();
                        static_cast<Worker*>(selectedUnit)->moveTo(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition()), map,
                            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
                        break;
                    default:
                        assert(false);
                    }
                }
            }
        }
    }

    selectedUnits.clear();
}

void FactionPlayer::instructWorkerReturnMinerals(const Map& map)
{
    for (auto& worker : m_workers)
    {
        if (worker.isSelected() && worker.isHoldingResources())
        {
            glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(worker.getPosition(), m_HQ.getAABB(), m_HQ.getPosition(), map);
            worker.moveTo(destination, map, [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
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
            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map, m_units, unit); });
    }
}

bool FactionPlayer::instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, int workerID)
{
    if (Globals::isPositionInMapBounds(position) && !map.isPositionOccupied(position))
    {
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
            return Faction::instructWorkerToBuild(entityType, position, map, *selectedWorker);
        }
    }

    return false;
}