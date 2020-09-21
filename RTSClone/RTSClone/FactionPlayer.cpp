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

namespace
{
    bool isOneUnitSelected(const std::list<Unit>& units, const std::list<Worker>& workers, const Entity** selectedEntity = nullptr)
    {
        int unitSelectedCount = 0;

        for (const auto& unit : units)
        {
            if (unit.isSelected())
            {
                if (selectedEntity)
                {
                    *selectedEntity = &unit;
                }
               
                ++unitSelectedCount;
                if (unitSelectedCount > 1)
                {
                    return false;
                }
            }
        }

        for (const auto& worker : workers)
        {
            if (worker.isSelected())
            {
                if (selectedEntity)
                {
                    *selectedEntity = &worker;
                }
               
                ++unitSelectedCount;
                if (unitSelectedCount > 1)
                {
                    return false;
                }
            }
        }

        return unitSelectedCount == 1;
    }

    bool isDoubleClick(const glm::vec3& mouseToGroundPosition, const glm::vec3& previousMousePosition)
    {
        return mouseToGroundPosition == previousMousePosition;
    }
}

FactionPlayer::FactionPlayer(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions)
    : Faction(factionController, hqStartingPosition, mineralPositions),
    m_selectionBox(),
    m_previousMouseToGroundPosition(),
    m_attackMoveSelected(false)
{}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map,
    const std::vector<const Faction*>& opposingFactions, EntityTarget& selectedTargetGUI)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            onLeftClick(window, camera, map, selectedTargetGUI);
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            onRightClick(window, camera, opposingFactions, map);
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
        onMouseMove(window, camera, selectedTargetGUI, map);
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

void FactionPlayer::handleEvent(const GameEvent& gameEvent, const Map& map)
{
    Faction::handleEvent(gameEvent, map);

    switch (gameEvent.type)
    {
    case eGameEventType::ActivatePlayerPlannedBuilding:
        m_plannedBuilding.set(gameEvent);
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

void FactionPlayer::moveSingularSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map)
{
    assert(isOneUnitSelected(m_units, m_workers));

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
    assert(m_selectedUnits.empty());

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
    
    if (!m_selectedUnits.empty())
    {
        assert(!isOneUnitSelected(m_units, m_workers));
        AABB selectionBoxAABB(m_selectedUnits);
        if (selectionBoxAABB.contains(mouseToGroundPosition))
        {
            std::vector<glm::vec3> unitFormationPositions = PathFinding::getInstance().getFormationPositions(mouseToGroundPosition,
                m_selectedUnits, map);

            if (unitFormationPositions.size() == m_selectedUnits.size())
            {
                for (int i = 0; i < unitFormationPositions.size(); ++i)
                {
                    m_selectedUnits[i]->moveTo(unitFormationPositions[i], map,
                        [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
                }
            }
        }
        else
        {
            auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
            {
                return mineral.getAABB().contains(mouseToGroundPosition);
            });

            if (mineralToHarvest != m_minerals.cend())
            {
                for(auto& selectedUnit : m_selectedUnits)
                {
                    if (selectedUnit->getEntityType() == eEntityType::Worker)
                    {
                        glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
                            mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map);

                        static_cast<Worker*>(selectedUnit)->moveTo(destination, map, 
                            [&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); },
                            eUnitState::MovingToMinerals, &(*mineralToHarvest));
                    }
                }
            }
            else
            {
                std::sort(m_selectedUnits.begin(), m_selectedUnits.end(), [](const auto& unitA, const auto& unitB)
                {
                    return glm::all(glm::lessThan(unitA->getPosition(), unitB->getPosition()));
                });

                glm::vec3 total(0.0f, 0.0f, 0.0f);
                for (const auto& selectedUnit : m_selectedUnits)
                {
                    total += selectedUnit->getPosition();
                }

                glm::vec3 averagePosition = { total.x / m_selectedUnits.size(), total.y / m_selectedUnits.size(), total.z / m_selectedUnits.size() };

                for (auto& selectedUnit : m_selectedUnits)
                {
                    switch (selectedUnit->getEntityType())
                    {
                    case eEntityType::Unit:
                    {
                        selectedUnit->resetTarget();
                        eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);

                        selectedUnit->moveTo(Globals::convertToNodePosition(mouseToGroundPosition - (averagePosition - selectedUnit->getPosition())), map,
                            [&](const glm::ivec2& position)
                        { return getAllAdjacentPositions(position, map, m_units, *selectedUnit, m_selectedUnits); }, state);
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

    m_selectedUnits.clear();
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

void FactionPlayer::onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, EntityTarget& selectedTargetGUI)
{
    bool selectAllUnits = false;
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
    if (!isDoubleClick(mouseToGroundPosition, m_previousMouseToGroundPosition))
    {
        m_previousMouseToGroundPosition = mouseToGroundPosition;

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
        selectedTargetGUI.reset();
    }

    m_selectionBox.setStartingPosition(window, mouseToGroundPosition);

    selectEntity<Unit>(m_units, mouseToGroundPosition, selectAllUnits);
    selectEntity<Worker>(m_workers, mouseToGroundPosition, selectAllUnits);
    selectEntity<Barracks>(m_barracks, mouseToGroundPosition);
    m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, 
    const std::vector<const Faction*>& opposingFactions, const Map& map)
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

        if (isOneUnitSelected(m_units, m_workers))
        {
            moveSingularSelectedUnit(mouseToGroundPosition, map);
        }
        else
        {
            moveMultipleSelectedUnits(mouseToGroundPosition, map);
        }
    }
}

void FactionPlayer::onMouseMove(const sf::Window& window, const Camera& camera, EntityTarget& selectedTargetGUI, const Map& map)
{
    if (m_selectionBox.active)
    {
        m_selectionBox.setSize(camera.getMouseToGroundPosition(window));
        selectUnits<Unit>(m_units, m_selectionBox);
        selectUnits<Worker>(m_workers, m_selectionBox);

        const Entity* selectedEntity = nullptr;
        if (isOneUnitSelected(m_units, m_workers, &selectedEntity))
        {
            assert(selectedEntity);
            selectedTargetGUI.set(getController(), selectedEntity->getID());
        }
        else
        {
            selectedTargetGUI.reset();
        }
    }
    else if (m_plannedBuilding.active)
    {
        glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
        if (Globals::isPositionInMapBounds(mouseToGroundPosition) && !map.isPositionOccupied(mouseToGroundPosition))
        {
            m_plannedBuilding.spawnPosition = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(mouseToGroundPosition));
        }
    }
}