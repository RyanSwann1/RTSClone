#pragma once

#include "glm/glm.hpp"
#include <functional>
#include <vector>

class Entity;
struct Base;
class Worker;
struct AIOccupiedBase
{
	AIOccupiedBase(const Base& base);

	bool isWorkerAdded(const Worker& worker) const;
	void addWorker(Worker& worker);
	void removeWorker(const Worker& worker);
	
	std::reference_wrapper<const Base> base;
	std::vector<std::reference_wrapper<Worker>> workers;
	std::vector<std::reference_wrapper<const Entity>> buildings;
	int turretCount;
	int barracksCount;
	int supplyDepotCount;
	int laboratoryCount;
};

class Headquarters;
class BaseHandler;
class AIOccupiedBases
{
public:
	AIOccupiedBases(const BaseHandler& baseHandler);
	AIOccupiedBases(const AIOccupiedBases&) = delete;
	AIOccupiedBases& operator=(const AIOccupiedBases&) = delete;
	AIOccupiedBases(AIOccupiedBases&&) = delete;
	AIOccupiedBases& operator=(AIOccupiedBases&&) = delete;

	const AIOccupiedBase& getBase(const Base& base) const;
	AIOccupiedBase& getBase(const Headquarters& headquarters);
	const std::vector<AIOccupiedBase>& getSortedBases(const glm::vec3& position);
	void addBase(const Base& base);
	void removeBase(const Base& base);

	void addWorker(Worker& worker, const Headquarters& headquarters);
	void addWorker(Worker& worker, const Base& base);
	void removeWorker(const Worker& worker);

	void addBuilding(const Worker& worker, const Entity& building);
	void removeBuilding(const Entity& building);

private:
	std::vector<AIOccupiedBase> m_bases;
	
	AIOccupiedBase* getBase(const Worker& worker);
};			