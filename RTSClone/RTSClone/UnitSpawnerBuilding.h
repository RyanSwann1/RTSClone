#pragma once

#include "Entity.h"
#include "Timer.h"
#include <functional>

class Map;
class Faction;
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

	void setWaypointPosition(const glm::vec3& position, const Map& map);
	void render(ShaderHandler& shaderHandler) const;

protected:
	UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime, int health, 
		const Faction& owningFaction);
	
	void update(float deltaTime, int resourceCost, int populationCost);

	const Faction& m_owningFaction;
	std::vector<std::function<Entity* (const UnitSpawnerBuilding&)>> m_unitsToSpawn;
	Timer m_spawnTimer;

private:
	glm::vec3 m_waypointPosition;
	
	void addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn);
};

class Barracks : public UnitSpawnerBuilding
{		
public:
	Barracks(const glm::vec3& startingPosition, const Faction& owningFaction);
	void update(float deltaTime);
};

class HQ : public UnitSpawnerBuilding
{
public:
	HQ(const glm::vec3& startingPosition, const Faction& owningFaction);
	void update(float deltaTime);
};