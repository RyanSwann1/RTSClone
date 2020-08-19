#pragma once

#include "UniqueEntityIDDistributer.h"
#include "UnitSpawnerBuilding.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include "Mineral.h"

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
	void addResources(Worker& worker);
	
	void update(float deltaTime, const Map& map);
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
	UnitSpawnerBuilding m_HQ;
	std::vector<Unit> m_units;
	std::vector<Worker> m_workers;
	std::vector<SupplyDepot> m_supplyDepots;
	std::vector<UnitSpawnerBuilding> m_barracks;

	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isEntityAffordable(eEntityType entityType) const;

	const Entity* addBuilding(Worker& worker, Map& map, glm::vec3 spawnPosition, eEntityType entityType);
	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map);
	void instructWorkerToBuild(eEntityType entityType, const glm::vec3& mouseToGroundPosition, Map& map);

	template <class Unit>
	void spawnUnit(Map& map, std::vector<Unit>& units, eEntityType entityType)
	{
		if (isEntityAffordable(entityType) && !isExceedPopulationLimit(entityType))
		{
			bool unitSpawned = false;
			switch (entityType)
			{
			case eEntityType::Unit:
			{
				auto barracks = std::find_if(m_barracks.begin(), m_barracks.end(), [](const auto& barracks)
				{
					return barracks.isSelected();
				});
				if (barracks != m_barracks.end())
				{
					if (barracks->isWaypointActive())
					{
						units.emplace_back(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(),
							Globals::convertToNodePosition(barracks->getUnitSpawnPosition()), PathFinding::getInstance().getClosestAvailablePosition(
							barracks->getWaypointPosition(), m_units, m_workers, map), map);
					}
					else
					{
						units.emplace_back(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(),
							Globals::convertToNodePosition(PathFinding::getInstance().getClosestAvailablePosition(barracks->getUnitSpawnPosition(),
							m_units, m_workers, map)));
					}

					unitSpawned = true;
				}
			}
			break;
			case eEntityType::Worker:
				if (m_HQ.isSelected())
				{
					if (m_HQ.isWaypointActive())
					{
						units.emplace_back(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(),
							m_HQ.getUnitSpawnPosition(), PathFinding::getInstance().getClosestAvailablePosition(
							m_HQ.getWaypointPosition(), m_units, m_workers, map), map);
					}
					else
					{
						units.emplace_back(UniqueEntityIDDistributer::getInstance().getUniqueEntityID(),
							PathFinding::getInstance().getClosestAvailablePosition(
							m_HQ.getUnitSpawnPosition(), m_units, m_workers, map));
					}

					unitSpawned = true;
				}
				break;
			default:
				assert(false);
			}

			if (unitSpawned)
			{
				reduceResources(entityType);
				increaseCurrentPopulationAmount(entityType);
			}
		}
	}

private:
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;

	template <class Entity>
	void handleCollisions(std::vector<Entity>& entities, const Map& map)
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