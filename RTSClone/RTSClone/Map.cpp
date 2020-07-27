#include "Map.h"
#include "Building.h"

//Map
Map::Map()
	: m_map()
{
	for (auto& i : m_map)
	{
		i = false;
	}
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	return m_map[Globals::convertTo1D(position)];
}

void Map::addBuilding(const Building& building)
{
	glm::ivec2 position(std::floor(building.getAABB().m_left), std::floor(building.getAABB().m_back));
	glm::ivec2 size(std::floor(building.getAABB().m_right - building.getAABB().m_left), 
		std::floor(building.getAABB().m_forward - building.getAABB().m_back));

	for (int x = position.x; x <= position.x + size.x; ++x)
	{
		for (int y = position.y; y <= position.y + size.y; ++y)
		{
			m_map[Globals::convertTo1D({ x, y })] = true;
		}
	}
}