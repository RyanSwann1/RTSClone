#include "AdjacentPositions.h"
#include "Map.h"
#include "Globals.h"
#include "Unit.h"
#include "Worker.h"
#include "FactionHandler.h"

const std::array<glm::ivec2, 8> ALL_DIRECTIONS_ON_GRID =
{
	glm::ivec2(0, 1),
	glm::ivec2(1, 1),
	glm::ivec2(1, 0),
	glm::ivec2(1, -1),
	glm::ivec2(0, -1),
	glm::ivec2(-1, -1),
	glm::ivec2(-1, 0),
	glm::ivec2(-1, 1)
};

AdjacentPosition::AdjacentPosition()
	: valid(false),
	position()
{}

AdjacentPosition::AdjacentPosition(glm::ivec2 position)
	: valid(true),
	position(position)
{}


AdjacentPositions createAdjacentPositions(const Map& map, const AABB& ignoreAABB)
{
	return [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, ignoreAABB); };
}

AdjacentPositions createAdjacentPositions(const Map& map)
{
	return [&](const glm::ivec2& position) { return getAdjacentPositions(position, map); };
}

AdjacentPositions createAdjacentPositions(const Map& map, FactionHandler& factionHandler, const Unit& unit)
{
	return [&](const glm::ivec2& position) { return getAdjacentPositions(position, map, factionHandler, unit); };
}

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2 & position,
	const Map & map, const EntitySpawnerBuilding& building)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && map.isPositionOnUnitMapAvailable(adjacentPosition, Globals::INVALID_ENTITY_ID) &&
			(building.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)) || !map.isPositionOccupied(adjacentPosition)))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getRandomAdjacentPositions(const glm::ivec2& position, 
	const Map& map, const AABB& ignoreAABB)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::array<glm::ivec2, 8> shuffledAllDirectionsOnGrid = ALL_DIRECTIONS_ON_GRID;
	std::shuffle(shuffledAllDirectionsOnGrid.begin(), shuffledAllDirectionsOnGrid.end(), g);

	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + shuffledAllDirectionsOnGrid[i];
		if (map.isWithinBounds(adjacentPosition))
		{
			if (ignoreAABB.contains(Globals::convertToWorldPosition(adjacentPosition)) ||
				!map.isPositionOccupied(adjacentPosition))
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getRandomAdjacentPositions(const glm::ivec2& position, const Map& map, const Unit& unit)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::array<glm::ivec2, 8> shuffledAllDirectionsOnGrid = ALL_DIRECTIONS_ON_GRID;
	std::shuffle(shuffledAllDirectionsOnGrid.begin(), shuffledAllDirectionsOnGrid.end(), g);

	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + shuffledAllDirectionsOnGrid[i];
		if (map.isWithinBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition) &&
			map.isPositionOnUnitMapAvailable(adjacentPosition, unit.getID()))
		{
  			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getAllAdjacentPositions(const glm::ivec2& position, const Map& map)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map, const AABB& ignoreAABB)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition))
		{
			if (ignoreAABB.contains(Globals::convertToWorldPosition(adjacentPosition)) || 
				!map.isPositionOccupied(adjacentPosition))
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}	
		}
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, 
	const Map& map, FactionHandler& factionHandler, const Unit& unit)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < static_cast<int>(ALL_DIRECTIONS_ON_GRID.size()); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && 
			!map.isPositionOccupied(adjacentPosition) &&
			map.isPositionOnUnitMapAvailable(adjacentPosition, unit.getID()))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}		
	}

	return adjacentPositions;
}

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map, 
	FactionHandler& factionHandler, const Unit& unit, const AABB& ignoreAABB)
{
	AdjacentPositionsContainer adjacentPositions;
	for (int i = 0; i < static_cast<int>(ALL_DIRECTIONS_ON_GRID.size()); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition))
		{
			if (ignoreAABB.contains(Globals::convertToWorldPosition(adjacentPosition)) || !map.isPositionOccupied(adjacentPosition))
			{
				if (map.isPositionOnUnitMapAvailable(adjacentPosition, unit.getID()))
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition);
				}
			}
		}
	}

	return adjacentPositions;
}