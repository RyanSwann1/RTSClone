#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "UniqueEntityIDDistributer.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition)
	: Entity(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(), startingPosition, eModelName::SupplyDepot, eEntityType::SupplyDepot)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}

SupplyDepot::~SupplyDepot()
{
	if (m_ID != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
}