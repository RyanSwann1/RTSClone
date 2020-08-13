#include "SupplyDepot.h"
#include "Map.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Entity(startingPosition, model, eEntityType::SupplyDepot)
{
	map.addEntityAABB(m_AABB);
}