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

FactionPlayer::FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation)
    : Faction(eFactionController::Player, hqStartingPosition, startingResources, startingPopulation),
    m_selected_entities(this)
{}

const std::vector<ConstSafePTR<Entity>>& FactionPlayer::getSelectedEntities() const
{
    return m_selected_entities.SelectedEntities();
}

void FactionPlayer::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, 
    const Map& map, FactionHandler& factionHandler, const HarvestLocationManager& harvest_locations, const MiniMap& miniMap, 
    const glm::vec3& levelSize)
{
    m_selected_entities.HandleInput(harvest_locations, camera, currentSFMLEvent, window, 
        miniMap, factionHandler, levelSize, map, m_entities);

    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            build_planned_building(map);
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            m_plannedBuilding.reset();
        }
        break;
    case sf::Event::MouseMoved:
        if (m_plannedBuilding)
        {
            m_plannedBuilding->handleInput(currentSFMLEvent, camera, window, map);
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
    {
        const auto entity = std::find_if(m_entities.all.cbegin(), m_entities.all.cend(),
                [id = gameEvent.data.playerActivatePlannedBuilding.targetID] (const auto& entity)
        {
            return entity->getID() == id;
        });
        if (entity != m_entities.all.cend())
        {
            m_plannedBuilding =
                std::optional<FactionPlayerPlannedBuilding>(std::in_place, 
                    gameEvent.data.playerActivatePlannedBuilding, (*entity)->getPosition(), this);
        }
    }
        break;
    case eGameEventType::PlayerSpawnEntity:
    {
        int targetEntityID = gameEvent.data.playerSpawnEntity.targetID;
        auto entity = std::find_if(m_entities.all.begin(), m_entities.all.end(), [targetEntityID](const auto& entity)
        {
            return entity->getID() == targetEntityID;
        });
        if (entity != m_entities.all.end())
        {
            (*entity)->AddEntityToSpawnQueue(*this);
        }
        break;
    }
    break;
    }
}

void FactionPlayer::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
    Faction::update(deltaTime, map, factionHandler);

    broadcast<GameMessages::UIDisplayPlayerDetails>(
        { getCurrentResourceAmount(), getCurrentPopulationAmount(), getMaximumPopulationAmount() });

    m_selected_entities.Update(m_entities);
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
    m_selected_entities.render(window, shaderHandler);
}

void FactionPlayer::on_entity_removal(const Entity& entity)
{
    Faction::on_entity_removal(entity);

    m_selected_entities.OnEntityRemoval(entity.getID());

    if (m_plannedBuilding &&
        m_plannedBuilding->getBuilderID() == entity.getID())
    {
        m_plannedBuilding.reset();
    }
}

void FactionPlayer::build_planned_building(const Map& map)
{
    if (!m_plannedBuilding)
    {
        return;
    }

    const auto selectedWorker = std::find_if(
        m_entities.workers.begin(), m_entities.workers.end(), [id = m_plannedBuilding->getBuilderID()](auto& worker)
    {
        return worker.getID() == id;
    });
    if (selectedWorker == m_entities.workers.cend()
        || !m_plannedBuilding->IsBuildingCreatable(map))
    {
        m_plannedBuilding.reset();
        return;
    }

    if ((*selectedWorker).build(*this, m_plannedBuilding->getPosition(), map, m_plannedBuilding->getEntityType()))
    {
        m_plannedBuilding.reset();
    }
}