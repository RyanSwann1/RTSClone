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
#include <functional>
#include <memory>

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
	const Headquarters& getMainHeadquarters() const;
	const Headquarters& getClosestHeadquarters(const glm::vec3& position) const;
	const glm::vec3& getMainHeadquartersPosition() const;
	eFactionController getController() const;
	const std::vector<std::unique_ptr<Unit>>& getUnits() const;
	const std::vector<std::reference_wrapper<Entity>>& getAllEntities() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID, eEntityType entityType) const;
	const Entity* getEntity(int entityID, eEntityType entityType) const;
	const Entity* getEntity(const glm::vec3& position) const;

	void addResources(Worker& worker);
	virtual void onUnitEnteredIdleState(Unit& unit, const Map& map, FactionHandler& factionHandler) {}
	virtual void onWorkerEnteredIdleState(Worker& worker, const Map& map) {}
	virtual void onUnitTakenDamage(const TakeDamageEvent& gameEvent, Unit& unit, const Map& map, FactionHandler& factionHandler) {}
	virtual Entity* createUnit(const Map& map, const Barracks& barracks, FactionHandler& factionHandler);
	virtual Entity* createWorker(const Map& map, const Headquarters& headquarters);
	virtual Entity* createBuilding(const Map& map, const Worker& worker);
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

	std::vector<std::reference_wrapper<Entity>> m_allEntities;
	std::vector<std::unique_ptr<Unit>> m_units;
	std::vector<std::unique_ptr<Worker>> m_workers;
	std::vector<std::unique_ptr<SupplyDepot>> m_supplyDepots;
	std::vector<std::unique_ptr<Barracks>> m_barracks;
	std::vector<std::unique_ptr<Turret>> m_turrets;
	std::vector<std::unique_ptr<Headquarters>> m_headquarters;
	std::vector<std::unique_ptr<Laboratory>> m_laboratories;

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
	void handleWorkerCollisions(const Map& map);

	//Presumes entity already found in all entities container
	template <class T>
	void removeEntity(std::vector<std::unique_ptr<T>>& entityContainer, int entityID, 
		std::vector<std::reference_wrapper<Entity>>::iterator entity)
	{
		assert(entity != m_allEntities.cend());
		
		auto iter = std::find_if(entityContainer.begin(), entityContainer.end(), [entityID](const auto& entity)
		{
			return entity->getID() == entityID;
		});
	
		assert(iter != entityContainer.end());

		onEntityRemoval((*entity).get());
		entityContainer.erase(iter);
		m_allEntities.erase(entity);
	}

	template <typename T>
	void removeEntity(std::vector<std::unique_ptr<T>>& entityContainer, int entityID)
	{
		auto iter = std::find_if(m_allEntities.begin(), m_allEntities.end(), [entityID](const auto& entity)
		{
			return entity.get().getID() == entityID;
		});
		if (iter != m_allEntities.end())
		{
			auto entity = std::find_if(entityContainer.begin(), entityContainer.end(), [entityID](const auto& entity)
			{
				return entity->getID() == entityID;
			});
			assert(entity != entityContainer.end());

			onEntityRemoval(*(*entity));
			m_allEntities.erase(iter);
			entityContainer.erase(entity);
		}
	}

	template <typename T>
	const Entity* getEntity(const T& entityContainer, int entityID) const
	{
		auto entity = std::find_if(entityContainer.cbegin(), entityContainer.cend(), [entityID](const auto& entity)
		{
			return entity->getID() == entityID;
		});

		return (entity != entityContainer.cend() ? &*(*entity) : nullptr);
	}

	template <typename T> 
	const Entity* getEntity(const T& entityContainer, int entityID, const AABB& entityAABB) const
	{
		auto entity = std::find_if(entityContainer.cbegin(), entityContainer.cend(), [&entityAABB, entityID](const auto& entity)
		{
			return entity.get()->getID() == entityID && entity.get()->getAABB().contains(entityAABB);
		});

		return (entity != entityContainer.cend() ? &*(*entity) : nullptr);
	}
};