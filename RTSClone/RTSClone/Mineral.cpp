#include "Mineral.h"
#include "Map.h"

Mineral::Mineral(const glm::vec3& startingPosition, Map& map)
	: Entity(startingPosition, eModelName::Mineral, eEntityType::Mineral)
{
	map.addEntityAABB(m_AABB);
}