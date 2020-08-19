#include "BuildingSpawner.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"
#include "ModelManager.h"
#include "GameEventMessenger.h"
#include "GameEvents.h"

namespace
{
	glm::vec3 getSpawnPosition(const AABB& AABB, const glm::vec3& direction, const glm::vec3& startingPosition)
	{
		glm::vec3 position = startingPosition;
		float distance = 1;
		while (AABB.contains(position))
		{
			position = position + direction * distance;
			++distance;
		}
		
		return position;
	}
}

BuildingSpawner::BuildingSpawner(const glm::vec3& startingPosition, eModelName modelName)
	: Entity(startingPosition, modelName, eEntityType::HQ),
	m_waypointPosition(m_position)
{
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::AddEntityToMap>>({ m_AABB });	
}

BuildingSpawner::BuildingSpawner(BuildingSpawner&& orig) noexcept
	: Entity(std::move(orig)),
	m_waypointPosition(orig.m_waypointPosition)
{}

BuildingSpawner& BuildingSpawner::operator=(BuildingSpawner&& orig) noexcept
{
	Entity::operator=(std::move(orig));
	m_waypointPosition = orig.m_waypointPosition;
	return *this;
}

BuildingSpawner::~BuildingSpawner()
{
	GameEventMessenger::getInstance().broadcast<GameEvents::MapModification<eGameEventType::RemoveEntityFromMap>>({ m_AABB });
}

bool BuildingSpawner::isWaypointActive() const
{
	assert(m_selected);
	return m_waypointPosition != m_position;
}

const glm::vec3& BuildingSpawner::getWaypointPosition() const
{
	assert(m_selected && isWaypointActive());
	return m_waypointPosition;
}

glm::vec3 BuildingSpawner::getUnitSpawnPosition() const
{
	assert(m_selected);
	if (isWaypointActive())
	{
		return getSpawnPosition(m_AABB, glm::normalize(m_waypointPosition - m_position), m_position);
	}
	else
	{
		return getSpawnPosition(m_AABB, 
			glm::normalize(glm::vec3(Globals::getRandomNumber(-1.0f, 1.0f), Globals::GROUND_HEIGHT, Globals::getRandomNumber(-1.0f, 1.0f))), 
			m_position);
	}
}

void BuildingSpawner::setWaypointPosition(const glm::vec3& position)
{
	assert(m_selected);
	if (Globals::isPositionInMapBounds(position))
	{
		if (m_AABB.contains(position))
		{
			m_waypointPosition = m_position;
		}
		else
		{
			m_waypointPosition = position;
		}
	}
}

void BuildingSpawner::render(ShaderHandler & shaderHandler) const
{
	if (m_selected && isWaypointActive())
	{
		ModelManager::getInstance().getModel(eModelName::Waypoint).render(shaderHandler, m_waypointPosition);
	}
	
	Entity::render(shaderHandler);
}