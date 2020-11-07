#pragma once

#include "Faction.h"
#include "Graph.h"
#include <queue>

enum class eActionType
{
	BuildSupplyDepot,
	BuildBarracks,
	BuildTurret
};

struct AIAction
{
	AIAction(eActionType actionType);
	AIAction(eActionType actionType, const glm::vec3& position);

	eActionType actionType;
	glm::vec3 position;
};

class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
		int startingPopulation);

	void setTargetFaction(const std::vector<const Faction*>& opposingFactions);
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler) override;

private:
	std::queue<eEntityType> m_spawnQueue;
	std::queue<AIAction> m_actionQueue;
	Timer m_delayTimer;
	Timer m_idleTimer;
	Timer m_spawnTimer;
	const Faction* m_targetFaction;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	const Mineral& getRandomMineral() const;
	Worker* getAvailableWorker(const glm::vec3& position);
	
	const Entity* spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType) override;
	const Entity* spawnUnit(const Map& map, const UnitSpawnerBuilding& building) override;
	const Entity* spawnWorker(const Map& map, const UnitSpawnerBuilding& building) override;

	void onBuild(const Map& map, eEntityType entityTypeToBuild, FactionHandler& factionHandler);
};