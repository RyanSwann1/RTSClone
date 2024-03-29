#pragma once

#include "Faction.h"
#include "Core/Graph.h"
#include "Core/Timer.h"
#include "AI/AIOccupiedBases.h"
#include "AI/AIAction.h"
#include "AI/AIConstants.h"
#include "AI/AIUnattachedToBaseWorkers.h"
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
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void selectEntity(const glm::vec3& position);
	Entity* createUnit(const EntityToSpawnFromBuilding& entity, const Map& map) override;
	Entity* createWorker(const EntityToSpawnFromBuilding& entity, const Map& map) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) override;

protected:
	void on_entity_removal(const Entity& entity) override;
	void on_entity_taken_damage(const TakeDamageEvent& gameEvent, Entity& entity, const Map& map, FactionHandler& factionHandler) override;
	void on_entity_idle(Entity& entity, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) override;

	Barracks* CreateBarracks(const WorkerScheduledBuilding& scheduled_building) override;
	Turret* CreateTurret(const WorkerScheduledBuilding& scheduled_building) override;
	Headquarters* CreateHeadquarters(const WorkerScheduledBuilding& scheduled_building) override;
	Laboratory* CreateLaboratory(const WorkerScheduledBuilding& scheduled_building) override;
	SupplyDepot* CreateSupplyDepot(const WorkerScheduledBuilding& scheduled_building) override;

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
	void on_unit_taken_damage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler);
	void on_unit_idle(Unit& unit, const Map& map, FactionHandler& factionHandler);
	void on_worker_idle(Worker& worker, const Map& map, const BaseHandler& baseHandler);
};