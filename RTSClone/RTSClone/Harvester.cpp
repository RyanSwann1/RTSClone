#include "Harvester.h"
#include "Map.h"

namespace
{
	glm::vec3 getClosestPositionFromMineral(const glm::vec3& harvesterPosition, const Entity& mineral, const Map& map)
	{
		glm::vec3 position = mineral.getPosition();
		float distance = 1.0f;
		while (mineral.getAABB().contains(position) && map.isPositionOccupied(position))
		{
			position = position + glm::normalize(harvesterPosition - mineral.getPosition()) * distance;
			distance++;
		}

		return position + glm::normalize(harvesterPosition - mineral.getPosition()) * 2.0f;
	}
}

Harvester::Harvester(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Unit(startingPosition, model, map),
	m_currentHarvesterState(eHarvesterState::BaseStateInUse)
{}

void Harvester::moveTo(const glm::vec3 & destinationPosition, const Map & map, const Entity & mineral)
{
	if (mineral.getAABB().contains(destinationPosition))
	{
		glm::vec3 position = getClosestPositionFromMineral(m_position, mineral, map);
		Unit::moveTo(position, map);
	}
	else
	{
		Unit::moveTo(destinationPosition, map);
	}
}