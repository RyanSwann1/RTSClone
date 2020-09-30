#pragma once

#include "UniqueEntityIDDistributer.h"
#include "UnitSpawnerBuilding.h"
#include "Worker.h"
#include "PathFinding.h"
#include "PathFindingLocator.h"
#include "SupplyDepot.h"
#include "Mineral.h"
#include "FactionController.h"
#include "PlannedBuilding.h"
#include <list>

struct GameEvent;
class FactionHandler;
class ShaderHandler;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	virtual ~Faction() {}

	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	const glm::vec3& getHQPosition() const;
	eFactionController getController() const;
	const std::list<Unit>& getUnits() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID) const;
	const Entity* getEntity(int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;
	const std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION>& getMinerals() const;

	virtual void handleEvent(const GameEvent& gameEvent, const Map& map);
	virtual void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	virtual void render(ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING
		
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions, int startingResources,
		int startingPopulation);
	std::array<Mineral, Globals::MAX_MINERALS_PER_FACTION> m_minerals;
	std::vector<PlannedBuilding> m_plannedBuildings;
	std::vector<Entity*> m_allEntities;
	std::list<Unit> m_units;
	std::list<Worker> m_workers;
	std::list<SupplyDepot> m_supplyDepots;
	std::list<Barracks> m_barracks;
	HQ m_HQ;

	bool isEntityAffordable(eEntityType entityType) const;

	const Entity* addBuilding(const Map& map, glm::vec3 position, eEntityType entityType);
	bool addUnitToSpawn(eEntityType unitType, const Map& map, UnitSpawnerBuilding& building);
	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);

	template <class Unit>
	Entity* spawnUnit(const Map& map, std::list<Unit>& units, eEntityType entityType, const UnitSpawnerBuilding& building)
	{
		if (isEntityAffordable(entityType) && !isExceedPopulationLimit(entityType))
		{
			switch (entityType)
			{
			case eEntityType::Unit:
				if (building.isWaypointActive())
				{
					units.emplace_back(*this, Globals::convertToNodePosition(building.getUnitSpawnPosition()), PathFindingLocator::get().getClosestAvailablePosition(
						building.getWaypointPosition(), m_units, m_workers, map), map);
				}
				else
				{
					units.emplace_back(*this, Globals::convertToNodePosition(PathFindingLocator::get().getClosestAvailablePosition(building.getUnitSpawnPosition(),
						m_units, m_workers, map)));
				}
				break;
			case eEntityType::Worker:
				if (building.isWaypointActive())
				{
					units.emplace_back(*this, building.getUnitSpawnPosition(), PathFindingLocator::get().getClosestAvailablePosition(
						building.getWaypointPosition(), m_units, m_workers, map), map);
				}
				else
				{
					units.emplace_back(*this, PathFindingLocator::get().getClosestAvailablePosition(
						building.getUnitSpawnPosition(), m_units, m_workers, map));
				}
				break;
			default:
				assert(false);
			}

			reduceResources(entityType);
			increaseCurrentPopulationAmount(entityType);
			m_allEntities.push_back(&units.back());

			return &units.back();
		}

		return nullptr;
	}

private:
	const eFactionController m_controller;
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;

	bool isExceedPopulationLimit(eEntityType entityType) const;

	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void decreaseCurrentPopulationAmount(const Entity& entity);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map);
	void addResources(Worker& worker);

	template <class Entity>
	void handleCollisions(std::list<Entity>& entities, const Map& map)
	{
		static std::vector<const Entity*> handledUnits;
		for (auto& entity : entities)
		{
			if (entity.getCurrentState() == eUnitState::Idle)
			{	
				if (map.isPositionOccupied(entity.getPosition()))
				{
					entity.moveTo(PathFindingLocator::get().getClosestAvailablePosition<Entity>(entity, entities, map), map,
						[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
				}
				else
				{
					for (const auto& otherEntity : entities)
					{
						if (&entity != &otherEntity &&
							std::find(handledUnits.cbegin(), handledUnits.cend(), &otherEntity) == handledUnits.cend() &&
							otherEntity.getCurrentState() == eUnitState::Idle &&
							entity.getAABB().contains(otherEntity.getAABB()))
						{
							entity.moveTo(PathFindingLocator::get().getClosestAvailablePosition<Entity>(entity, entities, map), map,
								[&](const glm::ivec2& position) { return getAdjacentPositions(position, map); });
							break;
						}
					}
				}
			}

			handledUnits.push_back(&entity);
		}

		handledUnits.clear();
	}
};