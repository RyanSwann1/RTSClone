#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition)
	: Entity(startingPosition, eEntityType::SupplyDepot, 10)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}

SupplyDepot::~SupplyDepot()
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
}