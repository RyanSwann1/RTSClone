#include "SupplyDepot.h"
#include "GameEventMessenger.h"
#include "GameEvents.h"

SupplyDepot::SupplyDepot(int ID, const glm::vec3& startingPosition)
	: Entity(ID, startingPosition, eModelName::SupplyDepot, eEntityType::SupplyDepot)
{
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::AddEntityToMap>>({ m_AABB });
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
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::RemoveEntityFromMap>>({ m_AABB });
}