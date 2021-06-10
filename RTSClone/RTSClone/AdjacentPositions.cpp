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
	used(false),
	position()
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position)
	: valid(true),
	used(false),
	position(position)
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position, bool valid)
	: valid(valid),
	used(false),
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2 & position,
	const Map & map, const std::vector<std::unique_ptr<Unit>> & units, const std::vector<std::unique_ptr<Worker>>& workers, const AABB& ignoreAABB)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && 
			ignoreAABB.contains(Globals::convertToWorldPosition(adjacentPosition)) || !map.isPositionOccupied(adjacentPosition))
		{
			bool unitCollision = false;
			for (const auto& unit : units)
			{
				if (unit->getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
				{
					unitCollision = true;
					break;
				}
			}

			if (!unitCollision)
			{
				for (const auto& harvester : workers)
				{
					if (harvester->getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
					{
						unitCollision = true;
						break;
					}
				}
			}

			if (!unitCollision)
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition, true);
			}
		}
	}

	return adjacentPositions;
}

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getRandomAdjacentPositions(const glm::ivec2& position, 
	const Map& map, const AABB& ignoreAABB)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::array<glm::ivec2, 8> shuffledAllDirectionsOnGrid = ALL_DIRECTIONS_ON_GRID;
	std::shuffle(shuffledAllDirectionsOnGrid.begin(), shuffledAllDirectionsOnGrid.end(), g);

	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map, const AABB& ignoreAABB)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, 
	const Map& map, FactionHandler& factionHandler, const Unit& unit)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map, 
	FactionHandler& factionHandler, const Unit& unit, const AABB& ignoreAABB)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
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