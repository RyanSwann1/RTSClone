#pragma once

#include "Entity.h"
#include "Timer.h"
#include <functional>

enum class eFactionController;
struct Model;
class Map;
class Faction;
class FactionHandler;
class EntitySpawnerBuilding : public Entity
{
public:
	virtual ~EntitySpawnerBuilding();

	const Timer& getSpawnTimer() const;
	int getCurrentSpawnCount() const;
	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	virtual bool addToSpawn() = 0;
	void setWaypointPosition(const glm::vec3& position, const Map& map);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

protected:
	EntitySpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime, int health, 
		Faction& owningFaction, const Model& model);
	
	void update(float deltaTime, int resourceCost, int populationCost, 
		const Map& map, FactionHandler& factionHandler);

	Faction& m_owningFaction;
	std::vector<eEntityType> m_spawnQueue;
	Timer m_spawnTimer;

private:
	glm::vec3 m_waypointPosition;
};

class Barracks : public EntitySpawnerBuilding
{		
public:
	Barracks(const glm::vec3& startingPosition, Faction& owningFaction);

	void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	bool addToSpawn() override;
};

class Headquarters : public EntitySpawnerBuilding
{
public:
	Headquarters(const glm::vec3& startingPosition, Faction& owningFaction);

	void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	bool addToSpawn() override;
};