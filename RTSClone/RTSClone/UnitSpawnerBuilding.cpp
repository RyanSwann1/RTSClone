#include "UnitSpawnerBuilding.h"
#include "Camera.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Model.h"
#include "Globals.h"
#include "ModelManager.h"
#include "Faction.h"
#include "Map.h"
#include "ShaderHandler.h"

namespace
{
	const float TIME_BETWEEN_WORKER_SPAWN = 2.0f;
	const float TIME_BETWEEN_UNIT_SPAWN = 3.0f;
	const size_t MAX_UNITS_SPAWNABLE = 5;

	const float HQ_PROGRESS_BAR_WIDTH = 150.0f;
	const float HQ_PROGRESS_BAR_YOFFSET = 220.0f;

	const float BARRACKS_PROGRESS_BAR_WIDTH = 100.0f;
	const float BARRACKS_PROGRESS_BAR_YOFFSET = 80.0f;
	
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

	bool isEntityAffordable(const Faction& owningFaction, int resourceCost, int populationCost)
	{
		return owningFaction.getCurrentResourceAmount() >= resourceCost &&
			owningFaction.getCurrentPopulationAmount() + populationCost <= owningFaction.getMaximumPopulationAmount();
	}

	bool isUnitSpawnable(int unitToSpawnCount, int resourceCost, int populationCost, const Faction& owningFaction)
	{
		return unitToSpawnCount * resourceCost + resourceCost <= owningFaction.getCurrentResourceAmount() &&
			unitToSpawnCount * populationCost + populationCost <= owningFaction.getMaximumPopulationAmount();
	}
}

//Barracks
Barracks::Barracks(const glm::vec3& startingPosition, Faction& owningFaction)
	: UnitSpawnerBuilding(startingPosition, eEntityType::Barracks, TIME_BETWEEN_UNIT_SPAWN, 
		Globals::BARRACKS_STARTING_HEALTH, owningFaction,
		ModelManager::getInstance().getModel(BARRACKS_MODEL_NAME))
{}

void Barracks::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
	UnitSpawnerBuilding::update(deltaTime, Globals::UNIT_RESOURCE_COST, Globals::UNIT_POPULATION_COST, 
		map, factionHandler);
}

bool Barracks::addToSpawn()
{
	if (isUnitSpawnable(static_cast<int>(m_spawnQueue.size()), Globals::UNIT_RESOURCE_COST, Globals::UNIT_POPULATION_COST, m_owningFaction) &&
		UnitSpawnerBuilding::addToSpawn())
	{
		m_spawnQueue.push_back(eEntityType::Unit);
		return true;
	}

	return false;
}

//HQ
Headquarters::Headquarters(const glm::vec3& startingPosition, Faction& owningFaction)
	: UnitSpawnerBuilding(startingPosition, eEntityType::Headquarters, TIME_BETWEEN_WORKER_SPAWN, 
		Globals::HQ_STARTING_HEALTH, owningFaction, ModelManager::getInstance().getModel(HQ_MODEL_NAME))
{}

void Headquarters::update(float deltaTime, const Map& map, FactionHandler& factionHandler)
{
	UnitSpawnerBuilding::update(deltaTime, Globals::WORKER_RESOURCE_COST, Globals::WORKER_POPULATION_COST,
		map, factionHandler);
}

bool Headquarters::addToSpawn()
{
	if (isUnitSpawnable(static_cast<int>(m_spawnQueue.size()), Globals::WORKER_RESOURCE_COST, Globals::WORKER_POPULATION_COST, m_owningFaction) &&
		UnitSpawnerBuilding::addToSpawn())
	{
		m_spawnQueue.push_back(eEntityType::Worker);
		return true;
	}

	return false;
}

UnitSpawnerBuilding::~UnitSpawnerBuilding()
{
	broadcastToMessenger<GameMessages::RemoveFromMap>({ m_AABB });
}

const Timer& UnitSpawnerBuilding::getSpawnTimer() const
{
	return m_spawnTimer;
}

int UnitSpawnerBuilding::getCurrentSpawnCount() const
{
	return static_cast<int>(m_spawnQueue.size());
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

bool UnitSpawnerBuilding::addToSpawn()
{
	eEntityType entityTypeToSpawn;
	switch (getEntityType())
	{
	case eEntityType::Barracks:
		entityTypeToSpawn = eEntityType::Unit;
		break;
	case eEntityType::Headquarters:
		entityTypeToSpawn = eEntityType::Worker;
		break;
	default:
		assert(false);
		break;
	}

	if (m_spawnQueue.size() < MAX_UNITS_SPAWNABLE && 
		m_owningFaction.isEntityAffordable(entityTypeToSpawn) &&
		!m_owningFaction.isExceedPopulationLimit(entityTypeToSpawn))
	{
		m_spawnTimer.setActive(true);
		return true;
	}

	return false;
}

void UnitSpawnerBuilding::setWaypointPosition(const glm::vec3& position, const Map& map)
{
	if (map.isWithinBounds(position))
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

void UnitSpawnerBuilding::update(float deltaTime, int resourceCost, int populationCost, 
	const Map& map, FactionHandler& factionHandler)
{
	Entity::update(deltaTime);
	m_spawnTimer.update(deltaTime);
	if (m_spawnTimer.isExpired() && !m_spawnQueue.empty())
	{
		m_spawnTimer.resetElaspedTime();
		
		eEntityType entityToSpawn = m_spawnQueue.back();
		const Entity* spawnedEntity = nullptr;
		switch (entityToSpawn)
		{
		case eEntityType::Unit:
			spawnedEntity = m_owningFaction.spawnUnit(map, *this, factionHandler);
			break;
		case eEntityType::Worker:
			spawnedEntity = m_owningFaction.spawnWorker(map, *this);
			break;
		case eEntityType::Headquarters:
		case eEntityType::SupplyDepot:
		case eEntityType::Barracks:
		case eEntityType::Turret:
		case eEntityType::Laboratory:
			assert(false);
			break;
		}

		if (!spawnedEntity)
		{
			m_spawnQueue.clear();
			m_spawnTimer.setActive(false);
		}
		else
		{
			m_spawnQueue.pop_back();
			if (m_spawnQueue.empty())
			{
				m_spawnTimer.setActive(false);
			}
			else if (!isEntityAffordable(m_owningFaction, resourceCost, populationCost))
			{
				m_spawnQueue.clear();
				m_spawnTimer.setActive(false);
			}
		}
	}
}

void UnitSpawnerBuilding::render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const
{
	if (isSelected() && isWaypointActive())
	{
		ModelManager::getInstance().getModel(WAYPOINT_MODEL_NAME).render(shaderHandler, m_waypointPosition);
	}

	Entity::render(shaderHandler, owningFactionController);
}

void UnitSpawnerBuilding::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_spawnTimer.isActive())
	{
		float currentTime = m_spawnTimer.getElaspedTime() / m_spawnTimer.getExpiredTime();
		float width = 0.0f;
		float yOffset = 0.0f;
		switch (getEntityType())
		{
		case eEntityType::Headquarters:
			width = HQ_PROGRESS_BAR_WIDTH;
			yOffset = HQ_PROGRESS_BAR_YOFFSET;
			break;
		case eEntityType::Barracks:
			width = BARRACKS_PROGRESS_BAR_WIDTH;
			yOffset = BARRACKS_PROGRESS_BAR_YOFFSET;
			break;
		default:
			assert(false);
		}

		m_statbarSprite.render(m_position, windowSize, width, width * currentTime, Globals::DEFAULT_PROGRESS_BAR_HEIGHT, yOffset,
			shaderHandler, camera, Globals::PROGRESS_BAR_COLOR);
	}
}

UnitSpawnerBuilding::UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, 
	float spawnTimerExpirationTime, int health, Faction& owningFaction, const Model& model)
	: Entity(model, startingPosition, entityType, health, owningFaction.getCurrentShieldAmount()),
	m_owningFaction(owningFaction),
	m_spawnQueue(),
	m_spawnTimer(spawnTimerExpirationTime, false),
	m_waypointPosition(m_position)
{
	broadcastToMessenger<GameMessages::AddToMap>({ m_AABB });
	m_spawnQueue.reserve(MAX_UNITS_SPAWNABLE);
}