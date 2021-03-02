#include "AdjacentPositions.h"
#include "Map.h"
#include "Globals.h"
#include "Unit.h"
#include "Worker.h"
#include "FactionHandler.h"

AdjacentPosition::AdjacentPosition()
	: valid(false),
	position()
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position)
	: valid(true),
	position(position)
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position, bool valid)
	: valid(valid),
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2 & position,
	const Map & map, const std::vector<Unit> & units, const std::vector<Worker>& workers, const AABB& ignoreAABB)
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
				if (unit.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
				{
					unitCollision = true;
					break;
				}
			}

			if (!unitCollision)
			{
				for (const auto& harvester : workers)
				{
					if (harvester.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
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
			PathFinding::getInstance().isUnitPositionAvailable(Globals::convertToWorldPosition(adjacentPosition), unit, factionHandler,
				unit.getOwningFaction()))
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
				if (PathFinding::getInstance().isUnitPositionAvailable(Globals::convertToWorldPosition(adjacentPosition), unit, factionHandler,
					unit.getOwningFaction()))
				{
					adjacentPositions[i] = AdjacentPosition(adjacentPosition);
				}
			}
		}
		//	!map.isPositionOccupied(adjacentPosition) &&
		//	PathFinding::getInstance().isUnitPositionAvailable(Globals::convertToWorldPosition(adjacentPosition), unit, factionHandler,
		//		unit.getOwningFaction()))
		//{
		//	
		//}
	}

	return adjacentPositions;
}
