#pragma once

#include "Faction.h"
#include "UI/EntitySelector.h"
#include <SFML/Graphics.hpp>
#include <optional>

class BaseHandler;
struct Camera;
struct PlayerActivatePlannedBuildingEvent;
class FactionPlayerPlannedBuilding
{	
public:
	FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position);

	bool isOnValidPosition(const BaseHandler& baseHandler, const Map& map) const;
	const glm::vec3& getPosition() const;
	int getBuilderID() const;
	eEntityType getEntityType() const;

	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map);
	void render(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const;

private:
	std::reference_wrapper<const Model> m_model;
	int m_builderID;
	eEntityType m_entityType;
	glm::vec3 m_position;
};

class MiniMap;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation);

	const std::vector<Entity*>& getSelectedEntities() const;

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		const FactionHandler& factionHandler, const BaseHandler& baseHandler, const MiniMap& miniMap, const glm::vec3& levelSize);
	void handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void update(float deltaTime, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void render(ShaderHandler& shaderHandler) const override;
	void renderPlannedBuilding(ShaderHandler& shaderHandler, const BaseHandler& baseHandler, const Map& map) const;
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	std::optional<FactionPlayerPlannedBuilding> m_plannedBuilding;
	EntitySelector m_entitySelector;
	glm::vec3 m_previousPlaneIntersection;
	bool m_attackMoveSelected;
	bool m_addToDestinationQueue;
	std::vector<Entity*> m_selectedEntities;

	void on_entity_removal(const Entity& entity) override;

	void instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters);
	int instructWorkerToBuild(const Map& map, const BaseHandler& baseHandler);
	void moveSingularSelectedEntity(const glm::vec3& destination, const Map& map, Entity& selectedEntity, const BaseHandler& baseHandler) const;
	void moveMultipleSelectedEntities(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler);

	void onLeftClick(const sf::Window& window, const Camera& camera, const Map& map, const BaseHandler& baseHandler);
	void onRightClick(const sf::Window& window, const Camera& camera, const FactionHandler& factionHandler, const Map& map, 
		const BaseHandler& baseHandler, const MiniMap& minimap, const glm::vec3& levelSize);

	template <typename Entity>
	void selectEntity(std::vector<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities = false,
		int selectEntityID = Globals::INVALID_ENTITY_ID);

	template <typename T>
	void selectEntities(std::vector<T>& units);

	template <typename T>
	void deselectEntities(std::vector<T>& entities);
};

template<typename Entity>
inline void FactionPlayer::selectEntity(std::vector<Entity>& entities, const glm::vec3& mouseToGroundPosition, bool selectAllEntities, int selectEntityID)
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

template <typename T>
void FactionPlayer::selectEntities(std::vector<T>& units)
{
	for (auto& unit : units)
	{
		unit.setSelected(m_entitySelector.getAABB().contains(unit.getAABB()));
	}	
}

template <typename T>
void FactionPlayer::deselectEntities(std::vector<T>& entities)
{
	for (auto& entity : entities)
	{
		entity.setSelected(false);
	}
}