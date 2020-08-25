#pragma once

#include "UniqueEntityIDDistributer.h"
#include "UnitSpawnerBuilding.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include "Mineral.h"
#include <list>

struct BuildingToSpawn
{
	BuildingToSpawn(int workerID, const glm::vec3& spawnPosition, eModelName modelName)
		: workerID(workerID),
		spawnPosition(spawnPosition),
		modelName(modelName)
	{}

	int workerID;
	glm::vec3 spawnPosition;
	eModelName modelName;
};

class ShaderHandler;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	const Entity* getEntity(int entityID) const;
	int getEntityIDAtPosition(const glm::vec3& position) const;

	void addResources(Worker& worker);	
	void update(float deltaTime, const Map& map, const Faction& opposingFaction);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Faction(const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition);
	std::vector<Mineral> m_minerals;
	std::vector<Entity*> m_allEntities;
	std::list<Unit> m_units;
	std::list<Worker> m_workers;
	std::list<SupplyDepot> m_supplyDepots;
	std::list<UnitSpawnerBuilding> m_barracks;
	UnitSpawnerBuilding m_HQ;

	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isEntityAffordable(eEntityType entityType) const;

	const Entity* addBuilding(Worker& worker, Map& map, glm::vec3 spawnPosition, eEntityType entityType);
	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map);
	void instructWorkerToBuild(eEntityType entityType, const glm::vec3& mouseToGroundPosition, Map& map);

	template <class Unit>
	Unit* spawnUnit(const Map& map, std::list<Unit>& units, eEntityType entityType, UnitSpawnerBuilding& building)
	{
		if (isEntityAffordable(entityType) && !isExceedPopulationLimit(entityType))
		{
			switch (entityType)
			{
			case eEntityType::Unit:
			{
				if (building.isWaypointActive())
				{
					units.emplace_back(Globals::convertToNodePosition(building.getUnitSpawnPosition()), PathFinding::getInstance().getClosestAvailablePosition(
							building.getWaypointPosition(), m_units, m_workers, map), map);
				}
				else
				{
					units.emplace_back(Globals::convertToNodePosition(PathFinding::getInstance().getClosestAvailablePosition(building.getUnitSpawnPosition(),
							m_units, m_workers, map)));
				}
			}
			break;
			case eEntityType::Worker:
				if (building.isWaypointActive())
				{
					units.emplace_back(building.getUnitSpawnPosition(), PathFinding::getInstance().getClosestAvailablePosition(
						building.getWaypointPosition(), m_units, m_workers, map), map);
				}
				else
				{
					units.emplace_back(PathFinding::getInstance().getClosestAvailablePosition(
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
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;

	template <class Entity>
	void handleCollisions(std::list<Entity>& entities, const Map& map)
	{
		static std::vector<const Entity*> handledUnits;
		for (auto& entity : entities)
		{
			if (entity.getCurrentState() == eUnitState::Idle)
			{
				for (const auto& otherEntity : entities)
				{
					if (&entity != &otherEntity &&
						std::find(handledUnits.cbegin(), handledUnits.cend(), &otherEntity) == handledUnits.cend() &&
						otherEntity.getCurrentState() == eUnitState::Idle &&
						entity.getAABB().contains(otherEntity.getAABB()))
					{
						entity.moveTo(PathFinding::getInstance().getClosestPositionOutsideAABB<Entity>(entity, entities, map), map);
						break;
					}
				}
			}

			handledUnits.push_back(&entity);
		}

		handledUnits.clear();
	}
};