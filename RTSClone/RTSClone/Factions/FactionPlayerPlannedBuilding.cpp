#include "FactionPlayerPlannedBuilding.h"
#include "Graphics/ModelManager.h"
#include "Events/GameEvents.h"
#include "Core/Globals.h"
#include "Core/Map.h"
#include "Core/Camera.h"

namespace
{
    constexpr float PLANNED_BUILDING_OPACITY = 0.3f;
    constexpr glm::vec3 VALID_PLANNED_BUILDING_COLOR{ 0.0f, 1.0f, 0.0f };
    constexpr glm::vec3 INVALID_PLANNED_BUILDING_COLOR{ 1.0f, 0.0f, 0.0f };
}

FactionPlayerPlannedBuilding::FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position)
    : m_model(ModelManager::getInstance().getModel(MODEL_NAMES[static_cast<int>(gameEvent.entityType)])),
    m_builderID(gameEvent.targetID),
    m_entityType(gameEvent.entityType),
    m_position(position, GridLockActive::True),
    m_aabb(m_position.Get(), m_model.get())
{}

const glm::vec3& FactionPlayerPlannedBuilding::getPosition() const
{
    return m_position.Get();
}

int FactionPlayerPlannedBuilding::getBuilderID() const
{
    return m_builderID;
}

eEntityType FactionPlayerPlannedBuilding::getEntityType() const
{
    return m_entityType;
}

void FactionPlayerPlannedBuilding::handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map)
{
    if (event.type == sf::Event::MouseMoved)
    {
        Position new_position{ camera.getRayToGroundPlaneIntersection(window), GridLockActive::True };
        AABB new_aabb{ new_position.Get(), m_model };
        if (map.isWithinBounds(new_aabb))
        {
            m_position = new_position;
            m_aabb = new_aabb;
        }
    }
}

void FactionPlayerPlannedBuilding::render(ShaderHandler& shaderHandler, const Map& map) const
{
    glm::vec3 color = (isOnValidPosition(map) ? VALID_PLANNED_BUILDING_COLOR : INVALID_PLANNED_BUILDING_COLOR);
    m_model.get().render(shaderHandler, m_position.Get(), color, PLANNED_BUILDING_OPACITY);
}

bool FactionPlayerPlannedBuilding::isOnValidPosition(const Map& map) const
{
    assert(map.isWithinBounds(m_aabb));
    return !map.isAABBOccupied(m_aabb);
}