#pragma once

#include "Faction.h"
#include "EntitySelector.h"
#include <SFML/Graphics.hpp>

struct Base;
struct Camera;
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
	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map,
		const std::vector<Base>& bases);
	void activate(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;

private:
	const Model* m_model;
	int m_workerID;
	glm::vec3 m_position;
	eEntityType m_entityType;
};

class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation);

	const std::vector<Entity*>& getSelectedEntities() const;

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		FactionHandler& factionHandler, const std::vector<Base>& bases);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;
	void render(ShaderHandler& shaderHandler) const override;
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	FactionPlayerPlannedBuilding m_plannedBuilding;
	EntitySelector m_entitySelector;
	glm::vec3 m_previousPlaneIntersection;
	bool m_attackMoveSelected;
	std::vector<Entity*> m_selectedEntities;

	void onEntityRemoval(const Entity& entity) override;

	void instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters);
	int instructWorkerToBuild(const Map& map);
	void moveSingularSelectedEntity(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, 
		FactionHandler& factionHandler, const std::vector<Base>& bases) const;
	void moveMultipleSelectedEntities(const glm::vec3& mouseToGroundPosition, const Map& map, FactionHandler& factionHandler,
		const std::vector<Base>& bases);

	void onLeftClick(const sf::Window& window, const Camera& camera, const Map& map);
	void onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map, 
		const std::vector<Base>& bases);

	template <class Entity>
	void selectEntity(std::list<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities = false,
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
	void selectEntities(std::list<Entity>& units)
	{
		for (auto& unit : units)
		{
			unit.setSelected(m_entitySelector.getAABB().contains(unit.getAABB()));
		}
	}

	template <class Entity>
	void deselectEntities(std::list<Entity>& entities)
	{
		for (auto& entity : entities)
		{
			entity.setSelected(false);
		}
	}
};