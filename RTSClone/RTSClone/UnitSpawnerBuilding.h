#pragma once

#include "Entity.h"
#include "Timer.h"
#include <functional>

class UnitSpawnerBuilding : public Entity, private NonMovable
{
	friend class Faction;
public:
	~UnitSpawnerBuilding();

	const Timer& getSpawnTimer() const;
	int getCurrentSpawnCount() const;
	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void setWaypointPosition(const glm::vec3& position);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler) const;

protected:
	UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime, int health);

private:
	Timer m_spawnTimer;
	glm::vec3 m_waypointPosition;
	std::vector<std::function<Entity*(const UnitSpawnerBuilding&)>> m_unitsToSpawn;

	void addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn);
};

class Barracks : public UnitSpawnerBuilding
{		
public:	
	Barracks(const glm::vec3& startingPosition);
};

class HQ : public UnitSpawnerBuilding
{
public:
	HQ(const glm::vec3& startingPosition);
};