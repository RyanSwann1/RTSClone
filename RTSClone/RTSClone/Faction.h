#pragma once

#include "UniqueEntityIDDistributer.h"
#include "UnitSpawnerBuilding.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include "Mineral.h"
#include "FactionController.h"
#include "PlannedBuilding.h"
#include "Turret.h"
#include "Laboratory.h"
#include <forward_list>
#include <vector>

struct Camera;
struct GameEvent;
class FactionHandler;
class ShaderHandler;
class Map;
class Faction : private NonCopyable, private NonMovable
{
public:
	virtual ~Faction() {}

	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	const glm::vec3& getHQPosition() const;
	eFactionController getController() const;
	const std::forward_list<Unit>& getUnits() const;
	const std::forward_list<Worker>& getWorkers() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID) const;
	const Entity* getEntity(int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;
	const std::vector<Mineral>& getMinerals() const;

	virtual void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler);
	virtual void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer);
	virtual void render(ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityHealthBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void selectEntity(const glm::vec3& position) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING
		
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		const std::vector<glm::vec3>& mineralPositions, int startingResources, int startingPopulationCap);
	const std::vector<Mineral> m_minerals;
	std::vector<PlannedBuilding> m_plannedBuildings;
	std::vector<Entity*> m_allEntities;
	std::forward_list<Unit> m_units;
	std::forward_list<Worker> m_workers;
	std::forward_list<SupplyDepot> m_supplyDepots;
	std::forward_list<Barracks> m_barracks;
	std::forward_list<Turret> m_turrets;
	std::forward_list<Laboratory> m_laboratories;
	HQ m_HQ;

	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isEntityAffordable(eEntityType entityType) const;

	bool addUnitToSpawn(eEntityType unitType, const Map& map, UnitSpawnerBuilding& building, FactionHandler& factionHandler);
	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	virtual void onEntityRemoval(const Entity& entity) {}
	virtual const Entity* spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType);
	virtual const Entity* spawnUnit(const Map& map, const UnitSpawnerBuilding& building, FactionHandler& factionHandler);
	virtual const Entity* spawnWorker(const Map& map, const UnitSpawnerBuilding& building);

private:
	const eFactionController m_controller;
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;

	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void decreaseCurrentPopulationAmount(const Entity& entity);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map, FactionHandler& factionHandler);
	void addResources(Worker& worker);

	void handleUnitCollisions(const Map& map, FactionHandler& factionHandler);
	void handleWorkerCollisions(const Map& map);

	//Presumes entity already found in all entities container
	template <class T>
	void removeEntity(std::forward_list<T>& entityContainer, int entityID, std::vector<Entity*>::iterator entity)
	{
		assert(entity != m_allEntities.cend());
		onEntityRemoval(*(*entity));

		entityContainer.remove_if([entityID](const auto& entity)
		{
			return entity.getID() == entityID;
		});

		m_allEntities.erase(entity);
	}
};