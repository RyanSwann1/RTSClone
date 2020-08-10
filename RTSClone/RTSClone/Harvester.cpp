#include "Harvester.h"
#include "Map.h"
#include "Headquarters.h"
#include "Mineral.h"
#include "ModelManager.h"
#include "PathFinding.h"

namespace
{
	constexpr float HARVEST_TIME = 2.0f;
	constexpr float MOVEMENT_SPEED = 7.5f;
}

Harvester::Harvester(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Unit(startingPosition, model, map, eEntityType::Harvester),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
{}

Harvester::Harvester(const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Model & model, Map & map)
	: Unit(startingPosition, model, map, eEntityType::Harvester),
	m_harvestTimer(HARVEST_TIME),
	m_mineralToHarvest(nullptr)
{
	Unit::moveTo(destinationPosition, map);
}

void Harvester::update(float deltaTime, const ModelManager& modelManager, const Headquarters& HQ, const Map& map, 
	const std::vector<Harvester>& harvesters)
{
	Unit::update(deltaTime, modelManager);

	switch (m_currentState)
	{
	case eUnitState::MovingToMinerals:
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.resetFromCentre(m_position, modelManager.getModel(m_modelName).sizeFromCentre);
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();

				if (m_pathToPosition.empty())
				{
					m_currentState = eUnitState::Harvesting;
				}
			}
		}
		break;
	case eUnitState::ReturningMineralsToHQ:
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.resetFromCentre(m_position, modelManager.getModel(m_modelName).sizeFromCentre);
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();

				if (m_pathToPosition.empty())
				{
					m_currentState = eUnitState::MovingToMinerals;
					assert(m_mineralToHarvest);

					m_pathToPosition.clear();
					PathFinding::getInstance().getPathToClosestPositionOutsideAABB(m_position, m_mineralToHarvest->getAABB(), 
						m_mineralToHarvest->getPosition(), map, m_pathToPosition);
				}
			}
		}
		break;
	case eUnitState::Harvesting:
		m_harvestTimer.setActive(true);
		m_harvestTimer.update(deltaTime);
		if (m_harvestTimer.isExpired())
		{
			m_harvestTimer.setActive(false);
			m_harvestTimer.resetElaspedTime();

			m_currentState = eUnitState::ReturningMineralsToHQ;
			m_pathToPosition.clear();
			PathFinding::getInstance().getPathToClosestPositionOutsideAABB(m_position, HQ.getAABB(), HQ.getPosition(), map, m_pathToPosition);
		}
		break;
	}
}

void Harvester::moveTo(const glm::vec3 & destinationPosition, const Map & map, const std::vector<Mineral>& minerals)
{
	for (const auto& mineral : minerals)
	{
		if (mineral.getAABB().contains(destinationPosition))
		{
			m_mineralToHarvest = &mineral;
			m_pathToPosition.clear();
			PathFinding::getInstance().getPathToClosestPositionOutsideAABB(m_position, m_mineralToHarvest->getAABB(), 
				m_mineralToHarvest->getPosition(), map, m_pathToPosition);
			m_currentState = eUnitState::MovingToMinerals;

			break;
		}
		else
		{
			m_mineralToHarvest = nullptr;
		}
	}

	if (!m_mineralToHarvest)
	{
		Unit::moveTo(destinationPosition, map);
	}
}

void Harvester::moveTo(const glm::vec3& destinationPosition, const Map& map)
{
	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, 
		[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });

	if (m_currentState == eUnitState::Idle)
	{
		m_currentState = eUnitState::Moving;
	}
}