#include "AdjacentPositions.h"
#include "Map.h"
#include "Globals.h"
#include "Unit.h"
#include "Worker.h"
#include "FactionHandler.h"

namespace
{	
	bool isUnitPositionAvailable(const glm::vec3& position, const Unit& senderUnit, FactionHandler& factionHandler)
	{
		for (const auto& opposingFaction : factionHandler.getOpposingFactions(senderUnit.getOwningFactionController()))
		{
			auto unit = std::find_if(opposingFaction.get().getUnits().cbegin(), opposingFaction.get().getUnits().cend(), [&position](const auto& unit)
			{
				if (COLLIDABLE_UNIT_STATES.isMatch(unit.getCurrentState()))
				{
					return unit.getAABB().contains(position);
				}
				else
				{
					return !unit.getPathToPosition().empty() && unit.getPathToPosition().front() == position;
				}
			});
			if (unit != opposingFaction.get().getUnits().cend())
			{
				return false;
			}
		}

		assert(factionHandler.isFactionActive(senderUnit.getOwningFactionController()));
		const Faction& owningFaction = factionHandler.getFaction(senderUnit.getOwningFactionController());
		int senderUnitID = senderUnit.getID();
		auto unit = std::find_if(owningFaction.getUnits().cbegin(), owningFaction.getUnits().cend(), [&position, senderUnitID](const auto& unit)
		{
			if (COLLIDABLE_UNIT_STATES.isMatch(unit.getCurrentState()))
			{
				return unit.getID() != senderUnitID && unit.getAABB().contains(position);
			}
			else
			{
				return unit.getID() != senderUnitID && !unit.getPathToPosition().empty() && unit.getPathToPosition().front() == position;
			}
		});
		if (unit != owningFaction.getUnits().cend())
		{
			return false;
		}

		return true;
	}
}

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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2 & position, 
	const Map & map, const std::forward_list<Unit> & units, const std::forward_list<Worker>& workers)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
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

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, 
	const Map& map, FactionHandler& factionHandler, const Unit& unit)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < static_cast<int>(ALL_DIRECTIONS_ON_GRID.size()); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (map.isWithinBounds(adjacentPosition) && 
			!map.isPositionOccupied(adjacentPosition) &&
			isUnitPositionAvailable(Globals::convertToWorldPosition(adjacentPosition), unit, factionHandler))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}		
	}

	return adjacentPositions;
}