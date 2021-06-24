#pragma once

#include "Faction.h"
#include "Graph.h"
#include "Timer.h"
#include "AIOccupiedBases.h"
#include "AIAction.h"
#include "AIConstants.h"
#include "AIUnattachedToBaseWorkers.h"
#include <queue>
#include <vector>
#include <functional>

struct AISquad
{
	std::vector<std::reference_wrapper<Unit>> units;
};

class BaseHandler;
class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap, AIConstants::eBehaviour behaviour, const BaseHandler& baseHandler);

	bool isWithinRangeOfBuildings(const glm::vec3& position, float distance) const;

	void onUnitEnteredIdleState(Unit& unit, const Map& map, FactionHandler& factionHandler) override;
	void onWorkerEnteredIdleState(Worker& worker, const Map& map) override;
	void onUnitTakenDamage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler) override;
	bool increaseShield(const Laboratory& laboratory) override;
	Entity* createBuilding(const Map& map, const Worker& worker) override;
	void setTargetFaction(FactionHandler& factionHandler);
	void onFactionElimination(FactionHandler& factionHandler, eFactionController eliminatedFaction);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void selectEntity(const glm::vec3& position);
	Entity* createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler) override;
	Entity* createWorker(const Map& map, const Headquarters& headquarters) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;

protected:
	void onEntityRemoval(const Entity& entity) override;

private:
	const BaseHandler& m_baseHandler;
	const AIConstants::eBehaviour m_behaviour;
	AIUnattachedToBaseWorkers m_unattachedToBaseWorkers;
	AIOccupiedBases m_occupiedBases;
	Timer m_baseExpansionTimer;
	Timer m_delayTimer;
	Timer m_spawnTimer;
	eFactionController m_targetFaction;
	std::vector<std::reference_wrapper<Unit>> m_unitsOnHold;
	std::vector<AISquad> m_squads;

	void instructWorkersToRepair(const Headquarters& HQ, const Map& map);
	Worker* getAvailableWorker(const glm::vec3& position);
	Worker* getAvailableWorker(const glm::vec3& position, AIOccupiedBase& occupiedBase);

	bool build(const Map& map, eEntityType entityType, AIOccupiedBase& occupiedBase, Worker* worker = nullptr);
	bool handleAction(const AIAction& action, const Map& map, AIOccupiedBase& occupiedBase);
};