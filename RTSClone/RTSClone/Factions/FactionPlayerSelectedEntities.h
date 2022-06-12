#pragma once

#include "UI/EntitySelectorBox.h"
#include "Core/FactionController.h"
#include <vector>

struct Camera;
class Entity;
class Map;
class BaseHandler;
class FactionPlayer;
class FactionHandler;
class Headquarters;
class MiniMap;
class FactionPlayerSelectedEntities
{
public:
	FactionPlayerSelectedEntities(const FactionPlayer* owning_faction);
	
	const std::vector<Entity*>& SelectedEntities() const;

	void OnEntityRemoval(const int id);
	void Update();

	void HandleInput(const BaseHandler& base_handler, const Camera& camera, 
		const sf::Event& sfml_event, const sf::Window& window, const MiniMap& minimap,
		FactionHandler& faction_handler, const glm::vec3& level_size,
		const Map& map);
	void render(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	bool Move(const glm::vec3& position, const Map& map);
	bool Repair(const glm::vec3& position, const Map& map);
	bool SetWaypoints(const glm::vec3& position, const Map& map);
	bool Harvest(const glm::vec3& destination, const Map& map, const BaseHandler& baseHandler);
	bool Attack(const glm::vec3& position, FactionHandler& factionHandler, const Map& map);
	bool ReturnMinerals(const glm::vec3& position, const Map& map);
	void SelectAllOfType(const glm::vec3& position);
	void SelectSingle(const glm::vec3& position);
	
	const FactionPlayer* m_owning_faction	{};
	EntitySelectorBox m_selection_box		{};
	std::vector<Entity*> m_entities			{};
	glm::vec3 m_previous_mouse_position		{};
	bool m_add_to_destinations_on_move		{ false };
	bool m_attack_move						{ false };
};