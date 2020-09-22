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
    void handleSelectedUnits(std::vector<Unit*>& selectedUnits, const glm::vec3& mouseToGroundPosition, const Map& map,
        const std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION>& minerals, bool attackMoveSelected, const std::list<Unit>& units,
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
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
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
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionOutsideAABB(selectedUnit->getPosition(),
                        (*selectedEntity)->getAABB(), (*selectedEntity)->getPosition(), map);

                    Worker& selectedWorker = static_cast<Worker&>(*selectedUnit);
                    selectedWorker.setRepairTargetEntity((*selectedEntity)->getID());

                    static_cast<Worker&>(*selectedUnit).moveTo(destination, map, 
                        [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                        eUnitState::MovingToRepairPosition);
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

    bool isOnlyOneEntitySelected(const std::vector<Entity*>& entities)
    {
        int entitySelectedCount = 0;
        for (const auto& entity : entities)
        {
            if (entity->isSelected())
            {
                ++entitySelectedCount;
                if (entitySelectedCount > 1)
                {
                    return false;
                }
            }
        }

        return entitySelectedCount == 1;
    }

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
        onMouseMove(window, camera, map);
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

void FactionPlayer::updateSelectionBox(EntityTarget& selectedTargetGUI)
{
    if (m_selectionBox.isActive() && m_selectionBox.isMinimumSize())
    {
        selectUnits<Unit>(m_units, m_selectionBox);
        selectUnits<Worker>(m_workers, m_selectionBox);

        const Entity* selectedEntity = nullptr;
        if (isOneUnitSelected(m_units, m_workers, &selectedEntity))
        {
            assert(selectedEntity);
            selectedTargetGUI.set(getController(), selectedEntity->getID());
        }
        else if(!isOnlyOneEntitySelected(m_allEntities))
        {
            selectedTargetGUI.reset();
        }
    }
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
       
        selectedUnit->moveTo(Globals::convertToNodePosition(mouseToGroundPosition), map,
            [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, m_units, *selectedUnit); }, 
            (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving));
    }
    else
    {
        auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [](const auto& worker) {
            return worker.isSelected() == true;
        });
        assert(selectedWorker != m_workers.end());
        selectedWorker->resetTarget();

        auto mineralToHarvest = std::find_if(m_minerals.cbegin(), m_minerals.cend(), [&mouseToGroundPosition](const auto& mineral)
        {
            return mineral.getAABB().contains(mouseToGroundPosition);
        });
        if (mineralToHarvest != m_minerals.cend())
        {
            selectedWorker->moveTo(PathFinding::getInstance().getClosestPositionOutsideAABB(selectedWorker->getPosition(),
                mineralToHarvest->getAABB(), mineralToHarvest->getPosition(), map), 
                map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eUnitState::MovingToMinerals, &(*mineralToHarvest));
            
            return;
        }

        int selectedWorkerID = selectedWorker->getID();
        auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&mouseToGroundPosition, selectedWorkerID](const auto& entity)
        {
            return entity->getAABB().contains(mouseToGroundPosition) && entity->getID() != selectedWorkerID;
        });
        if (selectedEntity != m_allEntities.cend() &&
            (*selectedEntity)->getHealth() < (*selectedEntity)->getMaximumHealth())
        {
            selectedWorker->setRepairTargetEntity((*selectedEntity)->getID());
            selectedWorker->moveTo(PathFinding::getInstance().getClosestPositionOutsideAABB(selectedWorker->getPosition(),
                (*selectedEntity)->getAABB(), (*selectedEntity)->getPosition(), map),
                map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                eUnitState::MovingToRepairPosition);
        }
        else
        {
            selectedWorker->moveTo(mouseToGroundPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
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
        handleSelectedUnits(m_selectedUnits, mouseToGroundPosition, map, m_minerals, m_attackMoveSelected, m_units, m_allEntities);
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
    if (Globals::isPositionInMapBounds(m_plannedBuilding.getPosition()) && !map.isPositionOccupied(m_plannedBuilding.getPosition()))
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

    return false;
}

void FactionPlayer::onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, EntityTarget& selectedTargetGUI)
{
    bool selectAllUnits = false;
    int keepEntityIDSelected = Globals::INVALID_ENTITY_ID;
    glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
    if (!isDoubleClick(mouseToGroundPosition, m_previousMouseToGroundPosition))
    {
        m_previousMouseToGroundPosition = mouseToGroundPosition;

        if (m_plannedBuilding.isActive())
        {
            if (instructWorkerToBuild(map))
            {
                keepEntityIDSelected = m_plannedBuilding.getWorkerID();
                m_plannedBuilding.setActive(false);
            }
        }
    }
    else
    {
        selectAllUnits = true;
    }

    if (keepEntityIDSelected != Globals::INVALID_ENTITY_ID)
    {
        selectedTargetGUI.set(getController(), keepEntityIDSelected);
    }
    
    m_selectionBox.setStartingPosition(window, mouseToGroundPosition);

    selectEntity<Unit>(m_units, mouseToGroundPosition, selectAllUnits);
    selectEntity<Worker>(m_workers, mouseToGroundPosition, selectAllUnits, keepEntityIDSelected);
    selectEntity<Barracks>(m_barracks, mouseToGroundPosition);
    m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
}

void FactionPlayer::onRightClick(const sf::Window& window, const Camera& camera, 
    const std::vector<const Faction*>& opposingFactions, const Map& map)
{
    m_plannedBuilding.setActive(false);
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

        if (m_HQ.getAABB().contains(mouseToGroundPosition))
        {
            instructWorkerReturnMinerals(map);
        }
    }
}

void FactionPlayer::onMouseMove(const sf::Window& window, const Camera& camera, const Map& map)
{
    if (m_selectionBox.isActive())
    {
        m_selectionBox.setSize(camera.getMouseToGroundPosition(window));

    }
    else if (m_plannedBuilding.isActive())
    {
        m_plannedBuilding.setPosition(camera.getMouseToGroundPosition(window), map);
    }
}