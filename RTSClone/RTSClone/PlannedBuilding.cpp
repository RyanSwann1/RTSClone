#include "PlannedBuilding.h"
#include "Globals.h"
#include "ModelManager.h"

PlannedBuilding::PlannedBuilding()
    : active(false),
    workerID(Globals::INVALID_ENTITY_ID),
    spawnPosition(),
    entityType()
{}

PlannedBuilding::PlannedBuilding(int workerID, const glm::vec3& spawnPosition, eEntityType entityType)
    : active(true),
    workerID(workerID),
    spawnPosition(Globals::convertToMiddleGridPosition(spawnPosition)),
    entityType(entityType)
{
    assert(entityType == eEntityType::Barracks || entityType == eEntityType::SupplyDepot);
}

void PlannedBuilding::render(ShaderHandler& shaderHandler) const
{
    if (active)
    {
        ModelManager::getInstance().getModel(entityType).render(shaderHandler, spawnPosition);
    }
}