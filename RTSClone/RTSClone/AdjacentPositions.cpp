#include "AdjacentPositions.h"
#include "Map.h"
#include "Globals.h"
#include "Unit.h"
#include "Worker.h"

AdjacentPosition::AdjacentPosition()
	: valid(false),
	unitCollision(false),
	position()
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position)
	: valid(true),
	unitCollision(false),
	position(position)
{}

AdjacentPosition::AdjacentPosition(const glm::ivec2 & position, bool valid, bool unitCollision)
	: valid(valid),
	unitCollision(unitCollision),
	position(position)
{}

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2 & position, 
	const Map & map, const std::list<Unit> & units,
	const std::list<Worker>& workers)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (Globals::isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
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
				adjacentPositions[i] = AdjacentPosition(adjacentPosition, true, unitCollision);
			}
			else
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition, true, unitCollision);
			}
		}
	}

	return adjacentPositions;
}

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map, 
	const std::list<Unit>& units, const Unit& unit)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (Globals::isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
		{
			bool unitCollision = false;
			for (const auto& otherUnit : units)
			{
				if (&otherUnit != &unit && otherUnit.getCurrentState() == eUnitState::Idle)
				{
					if (otherUnit.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
					{
						unitCollision = true;
						break;
					}
				}
			}

			if (!unitCollision)
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}
		}
	}

	return adjacentPositions;
}

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map, 
	const std::list<Unit>& units, const Unit& unit, const std::vector<Unit*>& selectedUnits)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < adjacentPositions.size(); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (Globals::isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
		{
			bool unitCollision = false;
			for (const auto& otherUnit : units)
			{
				if (&otherUnit != &unit &&
					std::find(selectedUnits.cbegin(), selectedUnits.cend(), &otherUnit) == selectedUnits.cend() &&
					otherUnit.getCurrentState() == eUnitState::Idle)
				{
					if (otherUnit.getAABB().contains(Globals::convertToWorldPosition(adjacentPosition)))
					{
						unitCollision = true;
						break;
					}
				}
			}

			if (!unitCollision)
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}
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
		if (Globals::isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
		{
			adjacentPositions[i] = AdjacentPosition(adjacentPosition);
		}
	}

	return adjacentPositions;
}

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, 
	const Map& map, const std::vector<glm::vec3>& previousAssignedPositions)
{
	std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> adjacentPositions;
	for (int i = 0; i < static_cast<int>(ALL_DIRECTIONS_ON_GRID.size()); ++i)
	{
		glm::ivec2 adjacentPosition = position + ALL_DIRECTIONS_ON_GRID[i];
		if (Globals::isPositionInMapBounds(adjacentPosition) && !map.isPositionOccupied(adjacentPosition))
		{
			auto previousAssignedPosition =
				std::find_if(previousAssignedPositions.cbegin(), previousAssignedPositions.cend(), [&](const auto& position)
			{
				return position == Globals::convertToWorldPosition(adjacentPosition);
			});
			if (previousAssignedPosition == previousAssignedPositions.cend())
			{
				adjacentPositions[i] = AdjacentPosition(adjacentPosition);
			}
		}		
	}

	return adjacentPositions;
}