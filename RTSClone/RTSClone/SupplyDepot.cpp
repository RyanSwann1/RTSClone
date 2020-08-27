#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition)
	: Entity(startingPosition, eEntityType::SupplyDepot)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}

SupplyDepot::~SupplyDepot()
{
	assert(getID() != Globals::INVALID_ENTITY_ID);
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
}