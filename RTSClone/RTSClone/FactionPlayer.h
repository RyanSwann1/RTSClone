#pragma once

#include "Faction.h"
#include "SelectionBox.h"
#include <SFML/Graphics.hpp>

struct PlayerActivatePlannedBuildingEvent;
class FactionPlayerPlannedBuilding
{	
public:
	FactionPlayerPlannedBuilding();

	const glm::vec3& getPosition() const;
	int getWorkerID() const;
	eEntityType getEntityType() const;
	bool isActive() const;

	void deactivate();
	void setPosition(const glm::vec3& newPosition, const Map& map);
	void activate(const PlayerActivatePlannedBuildingEvent& gameEvent);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;

private:
	const Model* m_model;
	int m_workerID;
	glm::vec3 m_position;
	eEntityType m_entityType;
};

struct Base;
struct Camera;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulationCap,
		const Base& currentBase);

	const std::vector<Entity*>& getSelectedEntities() const;

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		FactionHandler& factionHandler);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;
	void updateSelectionBox();
	void render(ShaderHandler& shaderHandler) const override;
	void renderSelectionBox(const sf::Window& window) const;

private:
	FactionPlayerPlannedBuilding m_plannedBuilding;
	SelectionBox m_selectionBox;
	glm::vec3 m_previousPlaneIntersection;
	bool m_attackMoveSelected;
	std::vector<Entity*> m_selectedEntities;
	const Base& m_currentBase;

	void onEntityRemoval(const Entity& entity) override;

	void instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters);
	int instructWorkerToBuild(const Map& map);
	void moveSingularSelectedEntity(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, FactionHandler& factionHandler) const;
	void moveMultipleSelectedEntities(const glm::vec3& mouseToGroundPosition, const Map& map, FactionHandler& factionHandler);

	void onLeftClick(const sf::Window& window, const Camera& camera, const Map& map);
	void onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map);

	template <class Entity>
	void selectEntity(std::forward_list<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities = false,
		int selectEntityID = Globals::INVALID_ENTITY_ID)
	{
		auto selectedEntity = std::find_if(entities.begin(), entities.end(), [&mouseToGroundPosition](const auto& entity)
		{
			return entity.getAABB().contains(mouseToGroundPosition);
		});
		if (selectedEntity != entities.end())
		{
			if (selectAllEntities)
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
				if (selectEntityID == entity.getID())
				{
					entity.setSelected(true);
				}
				else
				{
					entity.setSelected(false);
				}
			}
		}
	}

	template <class Entity>
	void selectUnits(std::forward_list<Entity>& units, const SelectionBox& selectionBox)
	{
		for (auto& unit : units)
		{
			unit.setSelected(m_selectionBox.getAABB().contains(unit.getAABB()));
		}
	}

	template <class Entity>
	void deselectEntities(std::forward_list<Entity>& entities)
	{
		for (auto& entity : entities)
		{
			entity.setSelected(false);
		}
	}
};