#pragma once

#include "Faction.h"
#include "UI/EntitySelector.h"
#include "FactionPlayerPlannedBuilding.h"
#include <SFML/Graphics.hpp>
#include <optional>

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

	void select_singular_entity(const glm::vec3& position);
	void select_entity_all_of_type(const glm::vec3& position);
	void onRightClick(const glm::vec3& position, const Camera& camera, const FactionHandler& factionHandler, const Map& map, 
		const BaseHandler& baseHandler);

	bool attack_entity(const glm::vec3& position, const FactionHandler& factionHandler, const Map& map);
	bool set_building_waypoints(const glm::vec3& position, const Map& map);
	void return_selected_workers_to_return_minerals(const glm::vec3& position, const Map& map);
	bool selected_workers_harvest(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler);
	bool repair_entity(const glm::vec3& position, const Map& map);
	bool MoveSelectedEntities(const glm::vec3& position, const Map& map);
};