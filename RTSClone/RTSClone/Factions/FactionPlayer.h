#pragma once

#include "Faction.h"
#include "UI/EntitySelectorBox.h"
#include "FactionPlayerPlannedBuilding.h"
#include "FactionPlayerSelectedEntities.h"
#include <SFML/Graphics.hpp>
#include <optional>

class MiniMap;
class FactionPlayer : public Faction
{
public:
	FactionPlayer(const glm::vec3& hqStartingPosition, int startingResources, int startingPopulation);

	const std::vector<Entity*>& getSelectedEntities() const;

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map, 
		FactionHandler& factionHandler, const BaseHandler& baseHandler, const MiniMap& miniMap, const glm::vec3& levelSize);
	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) override;
	void render(ShaderHandler& shaderHandler) const override;
	void renderPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const;
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	FactionPlayerSelectedEntities m_selected_entities;
	std::optional<FactionPlayerPlannedBuilding> m_plannedBuilding{};

	void on_entity_removal(const Entity& entity) override;
	void build_planned_building(const Map& map, const BaseHandler& baseHandler);
};