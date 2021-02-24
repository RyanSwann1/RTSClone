#pragma once

#include "Faction.h"
#include "EntitySelector.h"
#include <SFML/Graphics.hpp>

class BaseHandler;
struct Camera;
struct PlayerActivatePlannedBuildingEvent;
class FactionPlayerPlannedBuilding
{	
public:
	FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position);
	FactionPlayerPlannedBuilding(const FactionPlayerPlannedBuilding&) = delete;
	FactionPlayerPlannedBuilding& operator=(const FactionPlayerPlannedBuilding&) = delete;
	FactionPlayerPlannedBuilding(FactionPlayerPlannedBuilding&&) = delete;
	FactionPlayerPlannedBuilding& operator=(FactionPlayerPlannedBuilding&&) = delete;

	bool isOnValidPosition(const BaseHandler& baseHandler, const Map& map) const;
	const glm::vec3& getPosition() const;
	int getBuilderID() const;
	eEntityType getEntityType() const;

	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map);
	void render(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const;

private:
	const Model& m_model;
	const int m_builderID;
	const eEntityType m_entityType;
	glm::vec3 m_position;
};

class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation);

	const std::vector<Entity*>& getSelectedEntities() const;

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		FactionHandler& factionHandler, const BaseHandler& baseHandler);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer) override;
	void render(ShaderHandler& shaderHandler) const override;
	void renderPlannedBuilding(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const;
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	std::unique_ptr<FactionPlayerPlannedBuilding> m_plannedBuilding;
	EntitySelector m_entitySelector;
	glm::vec3 m_previousPlaneIntersection;
	bool m_attackMoveSelected;
	std::vector<Entity*> m_selectedEntities;

	void onEntityRemoval(const Entity& entity) override;

	void instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters);
	int instructWorkerToBuild(const Map& map, const BaseHandler& baseHandler);
	void moveSingularSelectedEntity(const glm::vec3& mouseToGroundPosition, const Map& map, Entity& selectedEntity, 
		FactionHandler& factionHandler, const BaseHandler& baseHandler) const;
	void moveMultipleSelectedEntities(const glm::vec3& mouseToGroundPosition, const Map& map, FactionHandler& factionHandler,
		const BaseHandler& baseHandler);

	void onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, const BaseHandler& baseHandler);
	void onRightClick(const sf::Window& window, const Camera& camera, FactionHandler& factionHandler, const Map& map, 
		const BaseHandler& baseHandler);

	template <class Entity>
	void selectEntity(std::vector<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities = false,
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
	void selectEntities(std::vector<Entity>& units)
	{
		for (auto& unit : units)
		{
			unit.setSelected(m_entitySelector.getAABB().contains(unit.getAABB()));
		}
	}

	template <class Entity>
	void deselectEntities(std::vector<Entity>& entities)
	{
		for (auto& entity : entities)
		{
			entity.setSelected(false);
		}
	}
};