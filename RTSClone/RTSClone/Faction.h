#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "BuildingSpawner.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include <SFML/Graphics.hpp>

struct SelectionBox : private NonMovable, private NonCopyable
{
	SelectionBox();
	~SelectionBox();

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void setSize(const glm::vec3& position);
	void reset();
	void render(const sf::Window& window) const;

	AABB AABB;
	bool active;
	glm::vec3 mouseToGroundPosition;
	glm::vec2 startingPositionScreenPosition;
	glm::vec3 startingPositionWorldPosition;
	unsigned int vaoID;
	unsigned int vboID;
};

struct BuildingCommand;
class Mineral;
class ShaderHandler;
struct Camera;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	Faction(Map& map);

	void addResources(Worker& worker);

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Map& map, 
		const std::vector<Mineral>& minerals, float deltaTime);
	void update(float deltaTime, const Map& map);
	void render(ShaderHandler& shaderHandler) const;
	void renderSelectionBox(const sf::Window& window) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	int m_currentResourceAmount;
	int m_currentPopulationAmount;
	int m_currentPopulationLimit;
	SelectionBox m_selectionBox;
	BuildingSpawner m_HQ;
	std::vector<Unit> m_units;
	std::vector<Worker> m_workers;
	std::vector<SupplyDepot> m_supplyDepots;
	std::vector<BuildingSpawner> m_barracks;
	glm::vec3 m_previousMouseToGroundPosition;

	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isEntityAffordable(eEntityType entityType) const;
	bool isOneUnitSelected() const;
	
	const Entity* addBuilding(Worker& worker, Map& map, glm::vec3 spawnPosition, eEntityType entityType);
	void moveSingularSelectedUnit(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals);
	void moveMultipleSelectedUnits(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals);
	void revalidateExistingUnitPaths(const Map& map);
	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void increasePopulationLimit();
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
						units.emplace_back(Globals::convertToNodePosition(barracks->getUnitSpawnPosition()), PathFinding::getInstance().getClosestAvailablePosition(
							barracks->getWaypointPosition(), m_units, m_workers, map), map);
					}
					else
					{
						units.emplace_back(Globals::convertToNodePosition(PathFinding::getInstance().getClosestAvailablePosition(barracks->getUnitSpawnPosition(),
							m_units, m_workers, map)), map);
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
						units.emplace_back(m_HQ.getUnitSpawnPosition(), PathFinding::getInstance().getClosestAvailablePosition(
							m_HQ.getWaypointPosition(), m_units, m_workers, map), map);
					}
					else
					{
						units.emplace_back(PathFinding::getInstance().getClosestAvailablePosition(
							m_HQ.getUnitSpawnPosition(), m_units, m_workers, map), map);
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

	template <class Entity>
	void selectUnit(std::vector<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllUnits)
	{
		auto selectedEntity = std::find_if(entities.begin(), entities.end(), [&mouseToGroundPosition](const auto& entity)
		{
			return entity.getAABB().contains(mouseToGroundPosition);
		});
		if (selectedEntity != entities.cend())
		{
			if (selectAllUnits)
			{
				for (auto& entity : entities)
				{
					entity.setSelected(true);
				}
			}
			else
			{
				for (auto& entity : entities)
				{
					entity.setSelected(entity.getAABB().contains(mouseToGroundPosition));
				}
			}
		}
		else
		{
			for (auto& entity : entities)
			{
				entity.setSelected(false);
			}
		}
	}
};