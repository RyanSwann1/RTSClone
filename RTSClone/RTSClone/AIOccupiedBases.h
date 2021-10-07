#pragma once

#include "glm/glm.hpp"
#include "AIAction.h"
#include <functional>
#include <vector>
#include <deque>
#include <memory.h>

enum class eEntityType;
enum class eFactionController;
class Entity;
struct Base;
class Worker;
struct AIOccupiedBase
{
	AIOccupiedBase(const Base& base);

	eFactionController getFactionController() const;
	bool isWorkerAdded(const Worker& worker) const;
	int getQueuedAIActionTypeCount(eAIActionType actionType) const;
	int getWorkerBuildQueueCount(eEntityType entityType) const;
	int getSpawnedEntityCount(eEntityType entityType) const;

	const Entity* getBuilding(const Entity& building) const;
	Entity* getBuilding(eEntityType entityType) const;
	void addWorker(Worker& worker);
	void removeWorker(const Worker& worker);
	const Entity* removeBuilding(const Entity& building);
	
	std::reference_wrapper<const Base> base;
	std::deque<AIAction> actionQueue;
	AIPriorityActionQueue actionPriorityQueue;
	std::vector<std::reference_wrapper<Worker>> workers;
	std::vector<std::reference_wrapper<Entity>> buildings;
	int turretCount;
	int barracksCount;
	int supplyDepotCount;
	int laboratoryCount;
};

class FactionAI;
class Headquarters;
class BaseHandler;
class AIUnattachedToBaseWorkers;
struct AIOccupiedBases
{
	AIOccupiedBases(const BaseHandler& baseHandler, eFactionController owningFaction);
	AIOccupiedBases(const AIOccupiedBases&) = delete;
	AIOccupiedBases& operator=(const AIOccupiedBases&) = delete;
	AIOccupiedBases(AIOccupiedBases&&) = delete;
	AIOccupiedBases& operator=(AIOccupiedBases&&) = delete;

	AIOccupiedBase* getBase(const glm::vec3& position);
	AIOccupiedBase* getBase(const Entity& entity);

	const std::vector<AIOccupiedBase>& getSortedBases(const glm::vec3& position);

	void addWorker(Worker& worker, const Headquarters& headquarters);
	void addWorker(Worker& worker, const Base& base);
	void removeWorker(const Worker& worker);

	AIOccupiedBase& addBase(const Base& base);
	std::vector<std::reference_wrapper<Worker>> removeBase(const Base& base);
	void addBuilding(const Worker& worker, Entity& building);
	void removeBuilding(const Entity& building);

	AIOccupiedBase* getBaseWithWorker(const Worker& worker);
	
	std::vector<AIOccupiedBase> bases;
	const eFactionController owningFaction;
};			