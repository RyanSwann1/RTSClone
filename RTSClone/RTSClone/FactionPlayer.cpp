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
#include <assert.h>
#include <array>
#include <algorithm>

namespace
{
    bool isOneUnitSelected(std::list<Unit>& units, std::list<Worker>& workers, Entity** selectedEntity = nullptr)
    {
        int unitSelectedCount = 0;

        for (auto& unit : units)
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

        for (auto& worker : workers)
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

    void moveSelectedUnit(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, const std::list<Unit>& units,
        const std::vector<Entity*>& entities, const std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION>& minerals, bool attackMoveSelected)
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
                selectedWorker.setRepairTargetEntity((*selectedEntity)->getID());
                selectedWorker.moveTo(PathFindingLocator::get().getClosestPositionOutsideAABB(selectedWorker.getPosition(),
                    (*selectedEntity)->getAABB(), (*selectedEntity)->getPosition(), map),
                    map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); },
                    eUnitState::MovingToRepairPosition);
            }
            else
            {
                selectedWorker.moveTo(mouseToGroundPosition, map, [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
            }
        }
    }

    void moveSelectedUnits(std::vector<Unit*>& selectedUnits, const glm::vec3& mouseToGroundPosition, const Map& map,
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

    bool isDoubleClick(const glm::vec3& mouseToGroundPosition, const glm::vec3& previousMousePosition)
    {
        return mouseToGroundPosition == previousMousePosition;
    }
}

FactionPlayer::FactionPlayer(eFactionController factionController, const glm::vec3& hqStartingPosition, 
    const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
    int startingPopulation)
    : Faction(factionController, hqStartingPosition, mineralPositions, startingResources, startingPopulation),
    m_selectionBox(),
    m_previousMouseToGroundPosition(),
    m_attackMoveSelected(false)
{}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map,
    const std::vector<const Faction*>& opposingFactions, TargetEntity& selectedTargetGUI)
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
    case eGameEventType::PlayerActivatePlannedBuilding:
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

void FactionPlayer::updateSelectionBox(TargetEntity& selectedTargetGUI)
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

void FactionPlayer::onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, TargetEntity& selectedTargetGUI)
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
    eFactionController targetEntityFaction;
    const Entity* targetEntity = nullptr;
    for (const auto& faction : opposingFactions)
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
        if (!isOneUnitSelected(m_units, m_workers))
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

        Entity* selectedEntity = nullptr;
        if (isOneUnitSelected(m_units, m_workers, &selectedEntity))
        {
            assert(selectedEntity);
            moveSelectedUnit(mouseToGroundPosition, map, *selectedEntity, m_units, m_allEntities, m_minerals, m_attackMoveSelected);
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