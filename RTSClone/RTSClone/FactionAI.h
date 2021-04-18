#pragma once

#include "Faction.h"
#include "Graph.h"
#include "Timer.h"
#include "AIOccupiedBases.h"
#include "PriorityQueue.h"
#include <queue>
#include <vector>
#include <functional>

enum class eAIBehaviour
{
	Defensive,
	Expansive,
	Aggressive,
	Max = Aggressive
};

enum class eActionType
{
	BuildSupplyDepot,
	BuildBarracks,
	BuildTurret,
	BuildLaboratory,
	SpawnUnit,
	SpawnWorker,
	IncreaseShield
};

struct AIAction
{
	AIAction(eActionType actionType, const glm::vec3& basePosition);

	eActionType actionType;
	glm::vec3 basePosition;
};

class AIUnattachedToBaseWorkers
{
public:
	AIUnattachedToBaseWorkers();
	AIUnattachedToBaseWorkers(const AIUnattachedToBaseWorkers&) = delete;
	AIUnattachedToBaseWorkers& operator=(const AIUnattachedToBaseWorkers&) = delete;
	AIUnattachedToBaseWorkers(AIUnattachedToBaseWorkers&&) = delete;
	AIUnattachedToBaseWorkers& operator=(AIUnattachedToBaseWorkers&&) = delete;
	
	bool isEmpty() const;
	Worker& getClosestWorker(const glm::vec3& position);

	void addWorker(Worker& worker);
	void remove(const Worker& worker);

private:
	std::vector<std::reference_wrapper<Worker>> m_unattachedToBaseWorkers;
};

struct AIPriorityAction
{
	AIPriorityAction(int weight, AIAction action);

	int weight;
	AIAction action;
};

const auto AIPriorityActionCompare = [](AIPriorityAction a, AIPriorityAction b) -> bool { return b.weight > a.weight; };
using AIPriorityActionQueue = std::priority_queue<AIPriorityAction, std::vector<AIPriorityAction>, decltype(AIPriorityActionCompare)>;

class BaseHandler;
class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap, const BaseHandler& baseHandler);

	bool isWithinRangeOfBuildings(const glm::vec3& position, float distance) const;

	void onUnitTakenDamage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler) const override;
	bool increaseShield(const Laboratory& laboratory) override;
	const Entity* createBuilding(const Map& map, const Worker& worker) override;
	void setTargetFaction(FactionHandler& factionHandler);
	void onFactionElimination(FactionHandler& factionHandler, eFactionController eliminatedFaction);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void selectEntity(const glm::vec3& position);
	const Entity* createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler) override;
	Entity* createWorker(const Map& map, const Headquarters& headquarters) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;

protected:
	void onEntityRemoval(const Entity& entity, bool forceDestroyed = false) override;

private:
	const BaseHandler& m_baseHandler;
	AIPriorityActionQueue m_ActionPriorityQueue;
	AIUnattachedToBaseWorkers m_unattachedToBaseWorkers;
	AIOccupiedBases m_occupiedBases;
	Timer m_baseExpansionTimer;
	eAIBehaviour m_currentBehaviour;
	std::queue<AIAction> m_actionQueue;
	Timer m_delayTimer;
	Timer m_spawnTimer;
	eFactionController m_targetFaction;

	void instructWorkersToRepair(const Headquarters& HQ, const Map& map);
	Worker* getAvailableWorker(const glm::vec3& position);

	bool build(const Map& map, eEntityType entityType, Worker* worker = nullptr);
};