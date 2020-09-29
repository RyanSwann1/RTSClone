#pragma once

#include "Entity.h"
#include "Timer.h"
#include <functional>

class Map;
class Faction;
class UnitSpawnerBuilding : public Entity, private NonMovable
{
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
	void addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn);

	const Faction& m_owningFaction;
	std::vector<std::function<Entity* (const UnitSpawnerBuilding&)>> m_unitsToSpawn;
	Timer m_spawnTimer;

private:
	glm::vec3 m_waypointPosition;
};

class Barracks : public UnitSpawnerBuilding
{		
	friend class Faction;
public:
	Barracks(const glm::vec3& startingPosition, const Faction& owningFaction);
	void update(float deltaTime);
	
private:
	void addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn);
};

class HQ : public UnitSpawnerBuilding
{
	friend class Faction;
public:
	HQ(const glm::vec3& startingPosition, const Faction& owningFaction);
	void update(float deltaTime);
	
private:
	void addUnitToSpawn(const std::function<Entity* (const UnitSpawnerBuilding&)>& unitToSpawn);
};