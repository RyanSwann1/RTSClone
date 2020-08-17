#include "SupplyDepot.h"
#include "Map.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition, Map& map)
	: Entity(startingPosition, eModelName::SupplyDepot, eEntityType::SupplyDepot)
{
	map.addEntityAABB(m_AABB);
}