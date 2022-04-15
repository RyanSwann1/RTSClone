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

using AISquad = std::vector<std::reference_wrapper<Unit>>;

class BaseHandler;
class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap, AIConstants::eBehaviour behaviour, const BaseHandler& baseHandler);

	bool isWithinRangeOfBuildings(const glm::vec3& position, float distance) const;
	
	bool increaseShield(const Laboratory& laboratory) override;
	void setTargetFaction(FactionHandler& factionHandler);
	void onFactionElimination(FactionHandler& factionHandler, eFactionController eliminatedFaction);
	void handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void selectEntity(const glm::vec3& position);
	Entity* createUnit(const Map& map, const EntitySpawnerBuilding& spawner) override;
	Entity* createWorker(const Map& map, const EntitySpawnerBuilding& spawner) override;
	void update(float deltaTime, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) override;

protected:
	void on_entity_removal(const Entity& entity) override;
	void on_entity_taken_damage(const TakeDamageEvent& gameEvent, Entity& entity, const Map& map, const FactionHandler& factionHandler) override;
	void on_entity_idle(Entity& entity, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	Entity* create_building(const Worker& worker, const Map& map) override;

private:
	const AIConstants::eBehaviour m_behaviour;
	AIUnattachedToBaseWorkers m_unattachedToBaseWorkers;
	AIOccupiedBases m_occupiedBases;
	Timer m_baseExpansionTimer;
	Timer m_delayTimer;
	Timer m_spawnTimer;
	eFactionController m_targetFaction;
	std::vector<Unit*> m_unitsOnHold;
	std::vector<AISquad> m_squads;

	void instructWorkersToRepair(const Entity& entity, const Map& map);
	Worker* getAvailableWorker(const glm::vec3& position);
	Worker* getAvailableWorker(const glm::vec3& position, AIOccupiedBase& occupiedBase);

	bool build(const Map& map, eEntityType entityType, AIOccupiedBase& occupiedBase, const BaseHandler& baseHandler, Worker* worker = nullptr);
	bool handleAction(const AIAction& action, const Map& map, AIOccupiedBase& occupiedBase, const BaseHandler& baseHandler);
	void on_unit_taken_damage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, const FactionHandler& factionHandler);
	void on_unit_idle(Unit& unit, const Map& map, const FactionHandler& factionHandler);
	void on_worker_idle(Worker& worker, const Map& map, const BaseHandler& baseHandler);
};