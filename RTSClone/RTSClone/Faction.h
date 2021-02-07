#pragma once

#include "Headquarters.h"
#include "Barracks.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include "Mineral.h"
#include "FactionController.h"
#include "Turret.h"
#include "Laboratory.h"
#include <vector>
#include <list>

struct Camera;
struct GameEvent;
class FactionHandler;
class ShaderHandler;
class Map;
class Faction : private NonCopyable, private NonMovable
{
public:
	virtual ~Faction() {}

	bool isMineralInUse(const Mineral& mineral) const;
	bool isExceedPopulationLimit(int populationAmount) const;
	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isAffordable(eEntityType entityType) const;
	bool isAffordable(int resourceAmount) const;
	int getCurrentShieldAmount() const;
	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	const Headquarters& getClosestHeadquarters(const glm::vec3& position) const;
	const glm::vec3& getMainHeadquartersPosition() const;
	eFactionController getController() const;
	const std::list<Unit>& getUnits() const;
	const std::list<Worker>& getWorkers() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID) const;
	const Entity* getEntity(int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;

	void addResources(Worker& worker);
	virtual const Entity* spawnUnit(const Map& map, const EntitySpawnerBuilding& building, FactionHandler& factionHandler);
	virtual const Entity* spawnWorker(const Map& map, const EntitySpawnerBuilding& building);
	virtual const Entity* spawnBuilding(const Map& map, glm::vec3 position, eEntityType entityType);
	virtual void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler);
	virtual void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer);
	virtual void render(ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING
		
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap);

	std::vector<Entity*> m_allEntities;
	std::list<Unit> m_units;
	std::list<Worker> m_workers;
	std::list<SupplyDepot> m_supplyDepots;
	std::list<Barracks> m_barracks;
	std::list<Turret> m_turrets;
	std::list<Headquarters> m_headquarters;
	std::list<Laboratory> m_laboratories;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	virtual void onEntityRemoval(const Entity& entity) {}

private:
	const eFactionController m_controller;
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;
	int m_currentShieldAmount;

	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void decreaseCurrentPopulationAmount(const Entity& entity);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map, FactionHandler& factionHandler);
	void increaseShield();

	void handleUnitCollisions(const Map& map, FactionHandler& factionHandler);
	void handleWorkerCollisions(const Map& map);

	//Presumes entity already found in all entities container
	template <class T>
	void removeEntity(std::list<T>& entityContainer, int entityID, std::vector<Entity*>::iterator entity)
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