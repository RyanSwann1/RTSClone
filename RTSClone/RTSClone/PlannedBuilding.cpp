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

PlannedBuilding::PlannedBuilding(int workerID, const glm::vec3& position, eEntityType entityType)
    : m_active(true),
    m_workerID(workerID),
    m_position(Globals::convertToMiddleGridPosition(position)),
    m_entityType(entityType)
{
    assert(entityType == eEntityType::Barracks || entityType == eEntityType::SupplyDepot);
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

void PlannedBuilding::set(const GameEvent& gameEvent)
{
    assert(gameEvent.entityType == eEntityType::Barracks || gameEvent.entityType == eEntityType::SupplyDepot);
    m_active = true;
    m_entityType = gameEvent.entityType;
    m_workerID = gameEvent.targetID;
}

void PlannedBuilding::render(ShaderHandler& shaderHandler) const
{
    if (m_active)
    {
        ModelManager::getInstance().getModel(m_entityType).render(shaderHandler, m_position);
    }
}