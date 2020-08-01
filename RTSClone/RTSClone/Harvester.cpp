#include "Harvester.h"
#include "Map.h"
#include "Headquarters.h"

namespace
{
	constexpr float HARVEST_TIME = 2.0f;

	glm::vec3 getClosestPositionFromAABB(const glm::vec3& harvesterPosition, const glm::vec3& centrePositionAABB, const AABB& AABB, const Map& map)
	{
		glm::vec3 position = centrePositionAABB;
		float distance = 1.0f;
		while (AABB.contains(position) && map.isPositionOccupied(position))
		{
			position = position + glm::normalize(harvesterPosition - centrePositionAABB) * distance;
			distance++;
		}

		return position + glm::normalize(harvesterPosition - centrePositionAABB);
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

		return position + glm::normalize(harvesterPosition - mineral.getPosition()) * 0.25f;
	}
}

Harvester::Harvester(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Unit(startingPosition, model, map),
	m_currentHarvesterState(eHarvesterState::InUseByBaseState),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
{}

Harvester::Harvester(const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Model & model, Map & map)
	: Unit(startingPosition, model, map),
	m_currentHarvesterState(eHarvesterState::InUseByBaseState),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
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
			m_currentHarvesterState = eHarvesterState::Harvesting;
		}
		break;
	case eHarvesterState::ReturningMineralsToHQ:
		if (m_pathToPosition.empty())
		{
			m_currentHarvesterState = eHarvesterState::MovingToMinerals;
			assert(m_mineralToHarvest);
			Unit::moveTo(getClosestPositionFromAABB(m_position, m_mineralToHarvest->getPosition(), m_mineralToHarvest->getAABB(), map), map);
		}
		break;
	case eHarvesterState::Harvesting:
		m_harvestTimer.setActive(true);
		m_harvestTimer.update(deltaTime);
		if (m_harvestTimer.isExpired())
		{
			m_currentHarvesterState = eHarvesterState::ReturningMineralsToHQ;
			Unit::moveTo(getClosestPositionFromAABB(m_position, HQ.getPosition(), HQ.getAABB(), map), map);

			m_harvestTimer.setActive(false);
			m_harvestTimer.resetElaspedTime();
		}
		break;
	}
}

void Harvester::moveTo(const glm::vec3 & destinationPosition, const Map & map, const std::vector<Entity>& minerals)
{
	for (const auto& mineral : minerals)
	{
		if (mineral.getAABB().contains(destinationPosition))
		{
			m_mineralToHarvest = &mineral;
			glm::vec3 position = getClosestPositionFromMineral(m_position, mineral, map);
			Unit::moveTo(position, map);
			m_currentHarvesterState = eHarvesterState::MovingToMinerals;
			m_currentState = eUnitState::InUseByDerivedState;
			break;
		}
		else
		{
			m_mineralToHarvest = nullptr;
		}
	}

	if (!m_mineralToHarvest)
	{
		m_currentHarvesterState = eHarvesterState::InUseByBaseState;
		m_currentState = eUnitState::Moving;
		Unit::moveTo(destinationPosition, map);
	}
}