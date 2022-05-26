#include "FactionPlayer.h"
#include "Core/Globals.h"
#include "glad/glad.h"
#include "Core/Camera.h"
#include "Core/Map.h"
#include "Graphics/ModelManager.h"
#include "Core/PathFinding.h"
#include "Core/Mineral.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Events/GameEvents.h"
#include "FactionHandler.h"
#include "Core/Level.h"
#include <assert.h>
#include <array>
#include <algorithm>

namespace
{
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

FactionPlayer::FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation)
    : Faction(eFactionController::Player, hqStartingPosition, startingResources, startingPopulation)
{}

const std::vector<Entity*>& FactionPlayer::getSelectedEntities() const
{
    return m_selectedEntities;
}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, 
    const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler, const MiniMap& miniMap, 
    const glm::vec3& levelSize)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            const glm::vec3 mousePosition = camera.getRayToGroundPlaneIntersection(window);
            m_entitySelector.setStartingPosition(window, mousePosition);
            m_selectedEntities.clear();
            if (m_plannedBuilding)
            {
                build_planned_building(map, baseHandler);
            }
            else
            {
                if (mousePosition == m_previousMousePosition)
                {
                    select_entity_all_of_type(mousePosition);
                }
                else
                {
                    select_singular_entity(mousePosition);
                }
                
                m_previousMousePosition = mousePosition;
            }
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            glm::vec3 position(0.0f);
            if (miniMap.isIntersecting(window))
            {
                position = miniMap.get_relative_intersecting_position(window, levelSize);
            }
            else
            {
                position = camera.getRayToGroundPlaneIntersection(window);
            }

            onRightClick(position, camera, factionHandler, map, baseHandler);
        }
        break;
    case sf::Event::MouseButtonReleased:
        m_attackMoveSelected = false;
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            if (m_entitySelector.isActive() && !m_selectedEntities.empty())
            {
                for (auto& entity : m_allEntities)
                {
                    if (entity->isSelected())
                    {
                        entity->setSelected(entity->is_group_selectable());
                    }
                }
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

void FactionPlayer::handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    Faction::handleEvent(gameEvent, map, factionHandler, baseHandler);

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
        auto entity = std::find_if(m_allEntities.begin(), m_allEntities.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_allEntities.end())
        {
            switch ((*entity)->getEntityType())
            {
            case eEntityType::Barracks:
                static_cast<Barracks&>(*(*entity)).add_entity_to_spawn_queue(*this);
                break;
            case eEntityType::Headquarters:
                static_cast<Headquarters&>(*(*entity)).add_entity_to_spawn_queue(*this);
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

void FactionPlayer::update(float deltaTime, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler)
{
    Faction::update(deltaTime, map, factionHandler, baseHandler);
    broadcast<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });

    if (m_entitySelector.isActive())
    {
        m_selectedEntities.clear();
        for (auto& entity : m_allEntities)
        {
            if (entity->is_group_selectable() 
                && entity->setSelected(m_entitySelector.getAABB().contains(entity->getAABB())))
            {
                m_selectedEntities.push_back(entity);
            }
        }
        
        if (m_selectedEntities.size() == 1)
        {
            Level::add_event(
                GameEvent::create<SetTargetEntityGUIEvent>({ getController(), m_selectedEntities.back()->getID(), m_selectedEntities.back()->getEntityType() }));
        }
        else if (m_selectedEntities.size() > 1)
        {
            Level::add_event(GameEvent::create<ResetTargetEntityGUIEvent>({}));
        }
    }
}

void FactionPlayer::render(ShaderHandler& shaderHandler) const
{
    Faction::render(shaderHandler);
}

void FactionPlayer::renderPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const
{
    if (m_plannedBuilding)
    {
        m_plannedBuilding->render(shaderHandler, map);
    }
}

void FactionPlayer::renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const
{
    m_entitySelector.render(window, shaderHandler);
}

void FactionPlayer::on_entity_removal(const Entity& entity)
{
    Faction::on_entity_removal(entity);

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
            worker.return_minerals_to_headquarters(headquarters, map);
        }
    }
}

void FactionPlayer::build_planned_building(const Map& map, const BaseHandler& baseHandler)
{
    assert(m_plannedBuilding);
    if (!m_plannedBuilding->isOnValidPosition(map)
        || !isAffordable(m_plannedBuilding->getEntityType()))
    {
        m_plannedBuilding.reset();
        return;
    }

    const auto selectedWorker = std::find_if(m_workers.begin(), m_workers.end(), [id = m_plannedBuilding->getBuilderID()](auto& worker)
    {
        return worker.getID() == id;
    });
    assert(selectedWorker != m_workers.cend());
    if (m_plannedBuilding->getEntityType() == eEntityType::Headquarters)
    {
        if (const Base* base = baseHandler.getBase(m_plannedBuilding->getPosition()))
        {
            if ((*selectedWorker).build(*this, base->getCenteredPosition(), map, m_plannedBuilding->getEntityType()))
            {
                m_plannedBuilding.reset();
            }
        }
    }
    else
    {
        if ((*selectedWorker).build(*this, m_plannedBuilding->getPosition(), map, m_plannedBuilding->getEntityType()))
        {
            m_plannedBuilding.reset();
        }
    }
}

void FactionPlayer::moveSingularSelectedEntity(const glm::vec3& destination, const Map& map, Entity& selectedEntity, const BaseHandler& baseHandler) const
{
    switch (selectedEntity.getEntityType())
    {
    case eEntityType::Unit:
    {
        eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
        Unit& unit = static_cast<Unit&>(selectedEntity);
        if (m_addToDestinationQueue)
        {
            unit.add_destination(destination, map);
        }
        else
        {
            unit.move_to(destination, map);
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
                selectedWorker.harvest(*mineralToHarvest, map);
            }
        }
        else
        {
            auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), 
                [&destination, &selectedWorker](const auto& entity)
            {
                return entity->getAABB().contains(destination) && entity->getID() != selectedWorker.getID();
            });
            if (selectedEntity != m_allEntities.cend() &&
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
                    selectedWorker.move_to(destination, map);
                }
            }
        }
    }
        break;
    default:
        assert(false);
    }
}

void FactionPlayer::moveMultipleSelectedEntities(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler)
{
    assert(!m_selectedEntities.empty());
    const Base* base = baseHandler.getBaseAtMineral(destination);
    if (base)
    {
        for (auto& selectedEntity : m_selectedEntities)
        {
            const Mineral* mineral = baseHandler.getNearestAvailableMineralAtBase(*this, *base, selectedEntity->getPosition());
            if (mineral)
            {
                selectedEntity->harvest(*mineral, map);
            }
        }
    }
    else
    {
        auto entityToRepair = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&destination](const auto& entity)
        {
            return entity->getAABB().contains(destination);
        });
        if (entityToRepair != m_allEntities.cend())
        {
            for (auto& selectedEntity : m_selectedEntities)
            {
                if ((*entityToRepair)->getID() != selectedEntity->getID() &&
                    (*entityToRepair)->getHealth() < (*entityToRepair)->getMaximumHealth())
                {
                    glm::vec3 destination = PathFinding::getInstance().getClosestPositionToAABB(selectedEntity->getPosition(),
                        (*entityToRepair)->getAABB(), map);

                    selectedEntity->repairEntity(*(*entityToRepair), map);
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
                        unit.add_destination(position, map);
                    }
                    else
                    {
                        unit.move_to(position, map);
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
                        worker.move_to(position, map);
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

void FactionPlayer::select_singular_entity(const glm::vec3& position)
{
    for (auto& entity : m_allEntities)
    {
        entity->setSelected(false);
    }

    auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (selectedEntity != m_allEntities.cend())
    {
        (*selectedEntity)->setSelected(true);
    }
}

void FactionPlayer::select_entity_all_of_type(const glm::vec3& position)
{
    for (auto& entity : m_allEntities)
    {
        entity->setSelected(false);
    }

    auto selectedEntity = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (selectedEntity == m_allEntities.cend())
    {
        return;
    }

    if ((*selectedEntity)->is_group_selectable())
    {
        for (const auto& entity : m_allEntities)
        {
            entity->setSelected(entity->getEntityType() == (*selectedEntity)->getEntityType());
        }
    }
    else
    {
        (*selectedEntity)->setSelected(true);
    }
}

void FactionPlayer::onRightClick(const glm::vec3& position, const Camera& camera, const FactionHandler& factionHandler, const Map& map,
    const BaseHandler& baseHandler)
{
    m_plannedBuilding.reset();

    if (attack_entity(position, factionHandler, map))
    {
        return;
    }

    if (set_building_waypoints(position, map))
    {
        return;
    }

    if (repair_entity(position, map))
    {
        return;
    }

    if (m_selectedEntities.size() == 1)
    {
        moveSingularSelectedEntity(position, map, *m_selectedEntities.front(), baseHandler);
    }
    else if (!m_selectedEntities.empty())
    {
        moveMultipleSelectedEntities(position, map, baseHandler);
    }
}

bool FactionPlayer::attack_entity(const glm::vec3& position, const FactionHandler& factionHandler, const Map& map)
{
    for (const Faction* opposingFaction : factionHandler.getOpposingFactions(getController()))
    {
        if (!opposingFaction)
        {
            continue;
        }

        const Entity* targetEntity = opposingFaction->getEntity(position);
        if (targetEntity)
        {
            for (auto& selectedEntity : m_selectedEntities)
            {
                selectedEntity->attack_entity(*targetEntity, opposingFaction->getController(), map);
            }

            return true;
        }
    }

    return false;
}

bool FactionPlayer::set_building_waypoints(const glm::vec3& position, const Map& map)
{
    bool waypoint_selected = false;
    for (auto& entity : m_allEntities)
    {
        if (entity->isSelected() && entity->set_waypoint_position(position, map))
        {
            waypoint_selected = true;
        }
    }

    return waypoint_selected;
}

void FactionPlayer::return_selected_workers_to_return_minerals(const glm::vec3& position, const Map& map)
{
    for (auto& headquarters : m_headquarters)
    {
        if (headquarters.getAABB().contains(position))
        {
            instructWorkerReturnMinerals(map, headquarters);
            break;
        }
    }
}

void FactionPlayer::selected_workers_harvest(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler)
{
    if (!m_selectedEntities.empty())
    {
        const Base* base = baseHandler.getBaseAtMineral(destination);
        if (base)
        {
            for (auto& selectedEntity : m_selectedEntities)
            {
                if (selectedEntity->getEntityType() == eEntityType::Worker)
                {
                    const Mineral* mineral = baseHandler.getNearestAvailableMineralAtBase(*this, *base, selectedEntity->getPosition());
                    if (mineral)
                    {
                        static_cast<Worker&>(*selectedEntity).harvest(*mineral, map);
                    }
                }
            }
        }
    }
}

bool FactionPlayer::repair_entity(const glm::vec3& position, const Map& map)
{
    auto entity_to_repair = std::find_if(m_allEntities.cbegin(), m_allEntities.cend(), [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (entity_to_repair == m_allEntities.cend())
    {
        return false;
    }

    for (auto& selectedEntity : m_selectedEntities)
    {
        if (selectedEntity->getID() != (*entity_to_repair)->getID())
        {
            selectedEntity->repairEntity((*selectedEntity), map);
        }
    }    

    return true;
}