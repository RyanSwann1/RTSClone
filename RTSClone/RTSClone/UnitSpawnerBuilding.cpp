#include "UnitSpawnerBuilding.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"
#include "ModelManager.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "UniqueEntityIDDistributer.h"

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

UnitSpawnerBuilding::UnitSpawnerBuilding(const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType)
	: Entity(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(), startingPosition, modelName, entityType),
	m_waypointPosition(m_position)
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });	
}

UnitSpawnerBuilding::~UnitSpawnerBuilding()
{
	if (getID() != Globals::INVALID_ENTITY_ID)
	{
		GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
	}
}

bool UnitSpawnerBuilding::isWaypointActive() const
{
	return m_waypointPosition != m_position;
}

const glm::vec3& UnitSpawnerBuilding::getWaypointPosition() const
{
	return m_waypointPosition;
}

glm::vec3 UnitSpawnerBuilding::getUnitSpawnPosition() const
{
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

void UnitSpawnerBuilding::setWaypointPosition(const glm::vec3& position)
{
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

void UnitSpawnerBuilding::render(ShaderHandler & shaderHandler) const
{
	if (isSelected() && isWaypointActive())
	{
		ModelManager::getInstance().getModel(eModelName::Waypoint).render(shaderHandler, m_waypointPosition);
	}
	
	Entity::render(shaderHandler);
}