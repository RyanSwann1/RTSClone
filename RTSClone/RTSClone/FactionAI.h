#pragma once

#include "Faction.h"
#include "Graph.h"
#include <queue>
#include <functional>

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

class BaseHandler;
class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap, const BaseHandler& baseHandler);

	bool isWithinDistanceOfBuildings(const glm::vec3& position, float distance) const;
	const Entity* createBuilding(const Map& map, const Worker& worker) override;
	void setTargetFaction(FactionHandler& factionHandler);
	void onFactionElimination(FactionHandler& factionHandler, eFactionController eliminatedFaction);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void selectEntity(const glm::vec3& position);
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;

private:
	const BaseHandler& m_baseHandler;
	std::queue<eEntityType> m_spawnQueue;
	std::queue<AIAction> m_actionQueue;
	Timer m_delayTimer;
	Timer m_spawnTimer;
	eFactionController m_targetFaction;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	void instructWorkersToRepair(const Headquarters& HQ, const Map& map);
	Worker* getAvailableWorker(const glm::vec3& position);
	const Entity* createUnit(const Map& map, const EntitySpawnerBuilding& building, FactionHandler& factionHandler) override;
	Entity* createWorker(const Map& map, const EntitySpawnerBuilding& building) override;

	bool build(const Map& map, eEntityType entityType);
	bool build(const Map& map, eEntityType entityType, Worker& worker);
};