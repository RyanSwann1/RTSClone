#include "UnitSpawnerBuilding.h"
#include "Camera.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Model.h"
#include "Globals.h"
#include "ModelManager.h"

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

	constexpr float TIME_BETWEEN_WORKER_SPAWN = 2.0f;
	constexpr float TIME_BETWEEN_UNIT_SPAWN = 3.0f;
	constexpr int MAX_UNITS_SPAWNABLE = 5;
}

//Barracks
Barracks::Barracks(const glm::vec3& startingPosition)
	: UnitSpawnerBuilding(startingPosition, eEntityType::Barracks, TIME_BETWEEN_UNIT_SPAWN, Globals::BARRACKS_STARTING_HEALTH)
{}

//HQ
HQ::HQ(const glm::vec3& startingPosition)
	: UnitSpawnerBuilding(startingPosition, eEntityType::HQ, TIME_BETWEEN_WORKER_SPAWN, Globals::HQ_STARTING_HEALTH)
{}

UnitSpawnerBuilding::~UnitSpawnerBuilding()
{
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>({ m_AABB });
}

const Timer& UnitSpawnerBuilding::getSpawnTimer() const
{
	return m_spawnTimer;
}

int UnitSpawnerBuilding::getCurrentSpawnCount() const
{
	return static_cast<int>(m_unitsToSpawn.size());
}

bool UnitSpawnerBuilding::isWaypointActive() const
{
	return m_waypointPosition != m_position;
}

const glm::vec3& UnitSpawnerBuilding::getWaypointPosition() const
{
	assert(isWaypointActive());
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

void UnitSpawnerBuilding::addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn)
{
	if (static_cast<int>(m_unitsToSpawn.size()) < MAX_UNITS_SPAWNABLE)
	{
		m_unitsToSpawn.push_back(unitToSpawn);
		m_spawnTimer.setActive(true);
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

void UnitSpawnerBuilding::update(float deltaTime)
{
	m_spawnTimer.update(deltaTime);
	if (m_spawnTimer.isExpired() && !m_unitsToSpawn.empty())
	{
		m_spawnTimer.resetElaspedTime();
		
		auto& unitToSpawn = m_unitsToSpawn.back();
		if (!unitToSpawn(*this))
		{
			m_unitsToSpawn.clear();
			m_spawnTimer.setActive(false);
		}
		else
		{
			m_unitsToSpawn.pop_back();

			if (m_unitsToSpawn.empty())
			{
				m_spawnTimer.setActive(false);
			}
		}
	}
}

void UnitSpawnerBuilding::render(ShaderHandler& shaderHandler) const
{
	if (isSelected() && isWaypointActive())
	{
		ModelManager::getInstance().getModel(eModelName::Waypoint).render(shaderHandler, m_waypointPosition);
	}

	Entity::render(shaderHandler);
}

UnitSpawnerBuilding::UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime, int health)
	: Entity(startingPosition, entityType, health),
	m_spawnTimer(spawnTimerExpirationTime, false),
	m_waypointPosition(m_position),
	m_unitsToSpawn()
{
	m_unitsToSpawn.reserve(static_cast<size_t>(MAX_UNITS_SPAWNABLE));
	GameMessenger::getInstance().broadcast<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>({ m_AABB });
}