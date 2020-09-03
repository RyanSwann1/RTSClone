#pragma once

#include "Entity.h"
#include "Timer.h"
#include <functional>

class UnitSpawnerBuilding : public Entity, private NonMovable
{
public:
	~UnitSpawnerBuilding();

	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void addUnitToSpawn(const std::function<Entity*(const UnitSpawnerBuilding&)>& unitToSpawn);
	void setWaypointPosition(const glm::vec3& position);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler) const;

protected:
	UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime);

private:
	Timer m_spawnTimer;
	glm::vec3 m_waypointPosition;
	std::vector<std::function<Entity*(const UnitSpawnerBuilding&)>> m_unitsToSpawn;
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