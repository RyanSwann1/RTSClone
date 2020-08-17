#include "Map.h"
#include "AABB.h"

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
	if (Globals::isPositionInMapBounds(position))
	{
		glm::ivec2 positionOnGrid = Globals::convertToGridPosition(position);
		return m_map[Globals::convertTo1D(positionOnGrid)];
	}

	return true;
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	if (Globals::isPositionInMapBounds(position))
	{
		return m_map[Globals::convertTo1D(position)];
	}

	return true;
}

void Map::addEntityAABB(const AABB& AABB)
{
	for (int x = AABB.m_left; x < AABB.m_right; ++x)
	{
		for (int y = AABB.m_back; y < AABB.m_forward; ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, 0.0f, y });
			assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid)] = true;
		}
	}
}