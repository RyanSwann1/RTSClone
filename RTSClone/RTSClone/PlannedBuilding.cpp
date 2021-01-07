#include "PlannedBuilding.h"
#include "Globals.h"
#include "ModelManager.h"
#include "GameEvent.h"
#include "Map.h"

PlannedBuilding::PlannedBuilding()
    : m_active(false),
    m_workerID(Globals::INVALID_ENTITY_ID),
    m_position(),
    m_entityType()
{}

const glm::vec3& PlannedBuilding::getPosition() const
{
    return m_position;
}

int PlannedBuilding::getWorkerID() const
{
    return m_workerID;
}

eEntityType PlannedBuilding::getEntityType() const
{
    return m_entityType;
}

bool PlannedBuilding::isActive() const
{
    return m_active;
}

void PlannedBuilding::deactivate()
{
    m_active = false;
    m_workerID = Globals::INVALID_ENTITY_ID;
}

void PlannedBuilding::setPosition(const glm::vec3& newPosition, const Map& map)
{
    assert(m_workerID != Globals::INVALID_ENTITY_ID);

    if (map.isWithinBounds(newPosition) && !map.isPositionOccupied(newPosition))
    {
        m_position = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(newPosition));
    }
}

void PlannedBuilding::activate(const PlayerActivatePlannedBuildingEvent& gameEvent)
{
    m_active = true;
    m_entityType = gameEvent.entityType;
    m_workerID = gameEvent.targetID;
}

void PlannedBuilding::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
    if (m_active)
    {
        switch (m_entityType)
        {
        case eEntityType::SupplyDepot:
            ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Barracks:
            ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Turret:
            ModelManager::getInstance().getModel(TURRET_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        case eEntityType::Laboratory:
            ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME).render(shaderHandler, m_position, owningFactionController);
            break;
        default:
            assert(false);
        }
    }
}