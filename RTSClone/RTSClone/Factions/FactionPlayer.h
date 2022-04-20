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

	bool isOnValidPosition(const Map& map) const;
	const glm::vec3& getPosition() const;
	int getBuilderID() const;
	eEntityType getEntityType() const;

	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map);
	void render(ShaderHandler& shaderHandler, const Map& map) const;

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
	void renderPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const;
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	std::optional<FactionPlayerPlannedBuilding> m_plannedBuilding	= {};
	EntitySelector m_entitySelector									= {};
	glm::vec3 m_previousMousePosition								= {};
	bool m_attackMoveSelected										= false;
	bool m_addToDestinationQueue									= false;
	std::vector<Entity*> m_selectedEntities							= {};

	void on_entity_removal(const Entity& entity) override;
	void instructWorkerReturnMinerals(const Map& map, const Headquarters& headquarters);
	void build_planned_building(const Map& map, const BaseHandler& baseHandler);
	void moveSingularSelectedEntity(const glm::vec3& destination, const Map& map, Entity& selectedEntity, const BaseHandler& baseHandler) const;
	void moveMultipleSelectedEntities(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler);
	void select_singular_entity(const glm::vec3& position);
	void select_entity_all_of_type(const glm::vec3& position);
	void onRightClick(const glm::vec3& position, const Camera& camera, const FactionHandler& factionHandler, const Map& map, 
		const BaseHandler& baseHandler);

	void attack_entity(const glm::vec3& position, const FactionHandler& factionHandler, const Map& map);
	void set_building_waypoints(const glm::vec3& position, const Map& map);
	void return_selected_workers_to_return_minerals(const glm::vec3& position, const Map& map);
	void selected_workers_harvest(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler);
	void repair_entity(const glm::vec3& position, const Map& map);

	template <typename T>
	bool selectEntity(std::vector<T>& entities, const glm::vec3& mouseToGroundPosition, bool selectAll = false,
		std::vector<Entity*>* selectedEntities = nullptr);

	template <typename T>
	bool selectEntities(std::vector<T>& units, std::vector<Entity*>* selectedEntities = nullptr);

	template <typename T>
	void deselectEntities(std::vector<T>& entities);
};

template<typename T>
inline bool FactionPlayer::selectEntity(std::vector<T>& entities, const glm::vec3& position, bool selectAll, 
	std::vector<Entity*>* selectedEntities)
{
	const auto selectedEntity = std::find_if(entities.begin(), entities.end(), [&position](const auto& entity)
	{
		return entity.getAABB().contains(position);
	});
	if (selectedEntity != entities.end())
	{
		if (selectAll)
		{
			for (auto& entity : entities)
			{
				entity.setSelected(true);
				if (selectedEntities)
				{
					selectedEntities->push_back(&*selectedEntity);
				}
			}
		}
		else
		{
			for (auto& entity : m_allEntities)
			{
				entity->setSelected(false);
			}

			selectedEntity->setSelected(true);
			if (selectedEntities)
			{
				selectedEntities->push_back(&*selectedEntity);
			}
		}

		return true;
	}

	return false;
}

template <typename T>
bool FactionPlayer::selectEntities(std::vector<T>& entities, std::vector<Entity*>* selectedEntities)
{
	bool selected = false;
	for (auto& entity : entities)
	{
		entity.setSelected(m_entitySelector.getAABB().contains(entity.getAABB()));
		if (entity.isSelected())
		{
			selected = true;
			if (selectedEntities)
			{
				selectedEntities->push_back(&entity);
			}
		}
	}

	return selected;
}

template <typename T>
void FactionPlayer::deselectEntities(std::vector<T>& entities)
{
	for (auto& entity : entities)
	{
		entity.setSelected(false);
	}
}