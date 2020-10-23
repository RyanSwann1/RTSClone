#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "ModelManager.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition)
	: Entity(ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME), startingPosition, eEntityType::SupplyDepot, Globals::SUPPLY_DEPOT_STARTING_HEALTH)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}

SupplyDepot::~SupplyDepot()
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
}