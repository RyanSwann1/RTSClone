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

struct Camera;
struct GameEvent;
class FactionHandler;
class ShaderHandler;
class Map;
class Faction
{
public:
	Faction(const Faction&) = delete;
	Faction& operator=(const Faction&) = delete;
	Faction(Faction&&) = delete;
	Faction& operator=(Faction&&) = delete;
	virtual ~Faction() {}

	bool isMineralInUse(const Mineral& mineral) const;
	bool isExceedPopulationLimit(int populationAmount) const;
	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isAffordable(eEntityType entityType) const;
	bool isAffordable(int resourceAmount) const;
	bool isCollidingWithWorkerBuildQueue(const AABB& AABB) const;
	int getCurrentShieldAmount() const;
	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	const Headquarters& getClosestHeadquarters(const glm::vec3& position) const;
	const glm::vec3& getMainHeadquartersPosition() const;
	eFactionController getController() const;
	const std::vector<Unit>& getUnits() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID) const;
	const Entity* getEntity(int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;

	void addResources(Worker& worker);
	virtual void onUnitTakenDamage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler) {}
	virtual const Entity* createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler);
	virtual Entity* createWorker(const Map& map, const Headquarters& headquarters);
	virtual const Entity* createBuilding(const Map& map, const Worker& worker);
	virtual bool increaseShield(const Laboratory& laboratory);
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
	std::vector<Unit> m_units;
	std::vector<Worker> m_workers;
	std::vector<SupplyDepot> m_supplyDepots;
	std::vector<Barracks> m_barracks;
	std::vector<Turret> m_turrets;
	std::vector<Headquarters> m_headquarters;
	std::vector<Laboratory> m_laboratories;

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

	void handleUnitCollisions(const Map& map, FactionHandler& factionHandler);
	void handleWorkerCollisions(const Map& map);

	//Presumes entity already found in all entities container
	template <class T>
	void removeEntity(std::vector<T>& entityContainer, int entityID, std::vector<Entity*>::iterator entity)
	{
		assert(entity != m_allEntities.cend());
		onEntityRemoval(*(*entity));

		auto iter = std::find_if(entityContainer.begin(), entityContainer.end(), [entityID](const auto& entity)
		{
			return entity.getID() == entityID;
		});
		
		assert(iter != entityContainer.end());
		entityContainer.erase(iter);

		m_allEntities.erase(entity);
	}
};