#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameEvents.h"
#include "Globals.h"

SupplyDepot::SupplyDepot(int ID, const glm::vec3& startingPosition)
	: Entity(ID, startingPosition, eModelName::SupplyDepot, eEntityType::SupplyDepot)
{
	GameMessenger::getInstance().broadcast<GameEvents::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}

SupplyDepot::SupplyDepot(SupplyDepot&& orig) noexcept
	: Entity(std::move(orig))
{}

SupplyDepot& SupplyDepot::operator=(SupplyDepot&& orig) noexcept
{
	Entity::operator=(std::move(orig));
	return *this;
}

SupplyDepot::~SupplyDepot()
{
	if (m_ID != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
}