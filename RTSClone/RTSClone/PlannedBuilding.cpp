#include "PlannedBuilding.h"
#include "Globals.h"
#include "ModelManager.h"
#include "GameEvent.h"
#include "Map.h"

namespace
{
    const TypeComparison<eEntityType> ALLOWED_ENTITY_TYPES(
        { eEntityType::Barracks, eEntityType::SupplyDepot, eEntityType::Turret });
}

PlannedBuilding::PlannedBuilding()
    : m_active(false),
    m_workerID(Globals::INVALID_ENTITY_ID),
    m_position(),
    m_entityType()
{}

PlannedBuilding::PlannedBuilding(int workerID, const glm::vec3& position, eEntityType entityType)
    : m_active(true),
    m_workerID(workerID),
    m_position(Globals::convertToMiddleGridPosition(position)),
    m_entityType(entityType)
{
    assert(ALLOWED_ENTITY_TYPES.isMatch(entityType));
}

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

void PlannedBuilding::setActive(bool active)
{
    m_active = active;
}

void PlannedBuilding::setPosition(const glm::vec3& newPosition, const Map& map)
{
    assert(m_workerID != Globals::INVALID_ENTITY_ID);

    if (map.isWithinBounds(newPosition) && !map.isPositionOccupied(newPosition))
    {
        m_position = Globals::convertToMiddleGridPosition(Globals::convertToNodePosition(newPosition));
    }
}

void PlannedBuilding::set(const GameEvent_1& gameEvent)
{
    assert(ALLOWED_ENTITY_TYPES.isMatch(gameEvent.entityType));
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
        default:
            assert(false);
        }
    }
}