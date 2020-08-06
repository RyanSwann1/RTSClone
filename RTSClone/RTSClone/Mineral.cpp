#include "Mineral.h"
#include "Map.h"

Mineral::Mineral(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Entity(startingPosition, model, eEntityType::Mineral)
{
	map.addEntityAABB(m_AABB);
}