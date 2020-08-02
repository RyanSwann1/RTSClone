#include "Map.h"
#include "Headquarters.h"

//Map
Map::Map()
	: m_map()
{
	for (auto& i : m_map)
	{
		i = false;
	}
}

bool Map::isPositionOccupied(const glm::vec3& position) const
{
	assert(Globals::isPositionInMapBounds(position));
	return m_map[Globals::convertTo1D({ position.x, position.z })];
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	assert(Globals::isPositionInMapBounds(position));
	return m_map[Globals::convertTo1D(position)];
}

void Map::addEntityAABB(const AABB& AABB)
{
	glm::ivec2 position(std::floor(AABB.m_left), std::floor(AABB.m_back));
	glm::ivec2 size(std::floor(AABB.m_right - AABB.m_left), std::floor(AABB.m_forward - AABB.m_back));

	for (int x = position.x; x <= position.x + size.x; ++x)
	{
		for (int y = position.y; y <= position.y + size.y; ++y)
		{
			assert(Globals::isPositionInMapBounds({ x, y }));
			m_map[Globals::convertTo1D({ x, y })] = true;
		}
	}
}