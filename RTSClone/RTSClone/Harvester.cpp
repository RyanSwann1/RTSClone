#include "Harvester.h"
#include "Map.h"
#include "Headquarters.h"

namespace
{
	glm::vec3 getClosestPositionFromAABB(const glm::vec3& harvesterPosition, const glm::vec3& centrePositionAABB, const AABB& AABB, const Map& map)
	{
		glm::vec3 position = centrePositionAABB;
		float distance = 1.0f;
		while (AABB.contains(position) && map.isPositionOccupied(position))
		{
			position = position + glm::normalize(harvesterPosition - centrePositionAABB) * distance;
			distance++;
		}

		return position + glm::normalize(harvesterPosition - centrePositionAABB) * 2.0f;
	}

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
	m_currentHarvesterState(eHarvesterState::InUseByBaseState)
{}

Harvester::Harvester(const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Model & model, Map & map)
	: Unit(startingPosition, model, map),
	m_currentHarvesterState(eHarvesterState::InUseByBaseState)
{
	Unit::moveTo(destinationPosition, map);
}

void Harvester::update(float deltaTime, const ModelManager& modelManager, const Headquarters& HQ, const Map& map)
{
	Unit::update(deltaTime, modelManager);

	switch (m_currentHarvesterState)
	{
	case eHarvesterState::MovingToMinerals:
		if (m_pathToPosition.empty())
		{
			std::cout << "Harvest\n";
			m_currentHarvesterState = eHarvesterState::ReturningMineralsToHQ;
			Unit::moveTo(getClosestPositionFromAABB(m_position, HQ.getPosition(), HQ.getAABB(), map), map);
		}
		break;
	case eHarvesterState::ReturningMineralsToHQ:
		if (m_pathToPosition.empty())
		{
			m_currentHarvesterState = eHarvesterState::InUseByBaseState;
			m_currentState = eUnitState::Idle;
		}
		break;
	}
}

void Harvester::moveTo(const glm::vec3 & destinationPosition, const Map & map, const Entity & mineral)
{
	if (mineral.getAABB().contains(destinationPosition))
	{
		glm::vec3 position = getClosestPositionFromMineral(m_position, mineral, map);
		Unit::moveTo(position, map);
		m_currentHarvesterState = eHarvesterState::MovingToMinerals;
		m_currentState = eUnitState::InUseByDerivedState;
	}
	else
	{
		m_currentHarvesterState = eHarvesterState::InUseByBaseState;
		m_currentState = eUnitState::Moving;
		Unit::moveTo(destinationPosition, map);
	}
}