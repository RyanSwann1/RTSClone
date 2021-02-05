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

	void setWaypointPosition(const glm::vec3& position, const Map& map);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;

protected:
	EntitySpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType, float spawnTimerExpirationTime, int health, 
		Faction& owningFaction, const Model& model, int maxEntityInSpawnQueue);
	
	bool isEntitySpawnable(int maxEntitiesInSpawnQueue, int resourceCost, int populationCost) const;
	void addEntityToSpawnQueue(eEntityType entityType);
	virtual const Entity* spawnEntity(const Map& map, FactionHandler& factionHandler) const = 0;

	void update(float deltaTime, int resourceCost, int populationCost, 
		int maxEntityInSpawnQueue, const Map& map, FactionHandler& factionHandler);

	Faction& m_owningFaction;
	std::vector<eEntityType> m_spawnQueue;
	Timer m_spawnTimer;

private:
	glm::vec3 m_waypointPosition;
};