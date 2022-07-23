#include "FactionPlayerSelectedEntities.h"
#include "Core/Base.h"
#include "FactionPlayer.h"
#include "FactionHandler.h"
#include "Core/Level.h"
#include "FactionEntities.h"

namespace
{
    glm::vec3 getAveragePosition(std::vector<ConstSafePTR<Entity>>& selectedEntities)
    {
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

FactionPlayerSelectedEntities::FactionPlayerSelectedEntities(const FactionPlayer* owning_faction)
    : m_owning_faction(owning_faction)
{}

const std::vector<ConstSafePTR<Entity>>& FactionPlayerSelectedEntities::SelectedEntities() const
{
    return m_entities;
}

void FactionPlayerSelectedEntities::Update(FactionEntities& faction_entities)
{
    if (m_selection_box.isActive())
    {
        m_entities.clear();
        for (auto& entity : faction_entities.all)
        {
            if (entity->is_group_selectable()
                && entity->setSelected(m_selection_box.getAABB().contains(entity->getAABB())))
            {
                m_entities.emplace_back(*entity);
            }
        }
    }

    if (m_entities.size() == 1)
    {
        Level::add_event(
            GameEvent::create<SetTargetEntityGUIEvent>({ m_owning_faction->getController(), 
                m_entities.back()->getID(), m_entities.back()->getEntityType() }));
    }
    else if (m_entities.size() > 1)
    {
        Level::add_event(GameEvent::create<ResetTargetEntityGUIEvent>({}));
    }
}

void FactionPlayerSelectedEntities::HandleInput(const HarvestLocationManager& harvest_locations, const Camera& camera,    
    const sf::Event& sfml_event, const sf::Window& window, const MiniMap& minimap,
    FactionHandler& faction_handler, const glm::vec3& level_size, const Map& map, FactionEntities& faction_entities)
{
    switch (sfml_event.type)
    {
    case sf::Event::MouseButtonPressed:
    {
        if (sfml_event.mouseButton.button == sf::Mouse::Left)
        {
            const glm::vec3 mouse_position = camera.getRayToGroundPlaneIntersection(window);
            m_selection_box.setStartingPosition(window, mouse_position);
            m_entities.clear();

            if (mouse_position == m_previous_mouse_position)
            {
                SelectSingle(mouse_position, faction_entities);
            }
            else
            {
                SelectAllOfType(mouse_position, faction_entities);
            }

            m_previous_mouse_position = mouse_position;
        }
        else if (sfml_event.mouseButton.button == sf::Mouse::Right)
        {
            glm::vec3 position(0.0f);
            if (minimap.isIntersecting(window))
            {
                position = minimap.get_relative_intersecting_position(window, level_size);
            }
            else
            {
                position = camera.getRayToGroundPlaneIntersection(window);
            }

            if (Attack(position, faction_handler, map))
            {
                break;
            }

            if (SetWaypoints(position, map))
            {
                break;
            }

            if (Repair(position, map))
            {
                break;
            }

            if (Harvest(position, map, harvest_locations))
            {
                break;
            }

            if (Move(position, map))
            {
                break;
            }
        }
    }
        break;
    case sf::Event::MouseButtonReleased:
        m_attack_move = false;
        if (sfml_event.mouseButton.button == sf::Mouse::Left)
        {
            if (m_selection_box.isActive() && !m_entities.empty())
            {
                for (auto& entity : faction_entities.all)
                {
                    if (entity->isSelected())
                    {
                        entity->setSelected(entity->is_group_selectable());
                    }
                }
            }
        }
        m_selection_box.reset();
        break;
    case sf::Event::MouseMoved:
        m_selection_box.update(camera, window);
        break;
    case sf::Event::KeyPressed:
        switch (sfml_event.key.code)
        {
        case sf::Keyboard::A:
            m_attack_move = true;
            break;
        case sf::Keyboard::LShift:
            m_add_to_destinations_on_move = true;
            break;
        }
        break;
    case sf::Event::KeyReleased:
        m_add_to_destinations_on_move = false;
        break;
    }
}

void FactionPlayerSelectedEntities::render(const sf::Window& window, ShaderHandler& shaderHandler) const
{
    m_selection_box.render(window, shaderHandler);
}

bool FactionPlayerSelectedEntities::Move(const glm::vec3& position, const Map& map)
{
    bool selected_entity_moved = false;
    const glm::vec3 averagePosition = getAveragePosition(m_entities);
    for (auto& selectedEntity : m_entities)
    {
        //todo:
        //eUnitState state = (m_attackMoveSelected ? eUnitState::AttackMoving : eUnitState::Moving);
        glm::vec3 destination = position - (averagePosition - selectedEntity->getPosition());
        selected_entity_moved = selectedEntity->MoveTo(destination, map, m_add_to_destinations_on_move);
    }

    return selected_entity_moved;
}

bool FactionPlayerSelectedEntities::Repair(const glm::vec3& position, const Map& map)
{
    auto entity_to_repair = std::find_if(m_owning_faction->getEntities().cbegin(), m_owning_faction->getEntities().cend(), 
        [&position](const auto& entity)
    {
        return entity->getAABB().contains(position);
    });
    if (entity_to_repair == m_owning_faction->getEntities().cend())
    {
        return false;
    }

    for (auto& selectedEntity : m_entities)
    {
        if (selectedEntity->getID() != (*entity_to_repair)->getID())
        {
            selectedEntity->repairEntity(*(*selectedEntity), map);
        }
    }

    return true;
}

bool FactionPlayerSelectedEntities::SetWaypoints(const glm::vec3& position, const Map& map)
{
    bool waypoint_selected = false;
    for (auto& entity : m_entities)
    {
        if (entity->set_waypoint_position(position, map))
        {
            waypoint_selected = true;
        }
    }

    return waypoint_selected;
}

bool FactionPlayerSelectedEntities::Harvest(const glm::vec3& destination, 
    const Map& map, const HarvestLocationManager& harvest_locations)
{
    bool selected_entity_harvested{ false };
    if (const Mineral * mineral{ harvest_locations.MineralAtPosition(destination) })
    {
        for (auto& selected_entity : m_entities)
        {
            selected_entity_harvested = selected_entity->Harvest(*mineral, map);
        }
    }

    return selected_entity_harvested;
}

bool FactionPlayerSelectedEntities::Attack(const glm::vec3& position, FactionHandler& factionHandler, const Map& map)
{
    for (const Faction* opposingFaction : factionHandler.GetOpposingFactions(m_owning_faction->getController()))
    {
        const Entity* targetEntity = opposingFaction->getEntity(position);
        if (targetEntity)
        {
            for (auto& selectedEntity : m_entities)
            {
                selectedEntity->attack_entity(*targetEntity, opposingFaction->getController(), map);
            }

            return true;
        }
    }

    return false;
}

bool FactionPlayerSelectedEntities::ReturnMinerals(const glm::vec3& position, const Map& map)
{
    const auto hq = std::find_if(m_owning_faction->GetHeadquarters().cbegin(), 
        m_owning_faction->GetHeadquarters().cend(), [&position](const auto& hq)
    {
        return hq.getAABB().contains(position);
    });
    if (hq == m_owning_faction->GetHeadquarters().cend())
    {
        return false;
    }

    for (auto& entity : m_entities)
    {
        entity->ReturnMineralsToHeadquarters(*(hq), map);
    }

    return true;
}

void FactionPlayerSelectedEntities::SelectAllOfType(const glm::vec3& position, FactionEntities& faction_entities)
{
    ConstSafePTR<Entity> selected_entity = nullptr;
    for (auto& entity : faction_entities.all)
    {
        if (entity->getAABB().contains(position))
        {
            selected_entity = *entity;
        }

        entity->setSelected(false);
    }

    if (!selected_entity)
    {
        return;
    }

    if (selected_entity->is_group_selectable())
    {
        for (auto& entity : faction_entities.all)
        {
            if (entity->setSelected(entity->getEntityType() == selected_entity->getEntityType()))
            {
                m_entities.emplace_back(*entity);
            }

        }
    }
    else
    {
        selected_entity->setSelected(true);
        m_entities.emplace_back(*selected_entity);
    }
}

void FactionPlayerSelectedEntities::SelectSingle(const glm::vec3& position, FactionEntities& faction_entities)
{
    bool entity_selected = false;
    for (auto& entity : faction_entities.all)
    {
        if (!entity_selected && entity->getAABB().contains(position))
        {
            entity->setSelected(true);
            entity_selected = true;
            m_entities.emplace_back(*entity);
        }
        else
        {
            entity->setSelected(false);
        }
    }
}

void FactionPlayerSelectedEntities::OnEntityRemoval(const int id)
{
    auto entity = std::find_if(m_entities.begin(), m_entities.end(), [id](const auto& entity)
    {
        return entity->getID() == id;
    });

    m_entities.erase(entity, m_entities.end());
}