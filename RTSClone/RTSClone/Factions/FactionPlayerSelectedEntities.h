#pragma once

#include "UI/EntitySelectorBox.h"
#include "Core/FactionController.h"
#include "Core/ConstSafePTR.h"
#include <vector>

struct Camera;
class Entity;
class Map;
class FactionPlayer;
class FactionHandler;
class Headquarters;
class MiniMap;
class HarvestLocationManager;
struct FactionEntities;
class FactionPlayerSelectedEntities
{
public:
	FactionPlayerSelectedEntities(const FactionPlayer* owning_faction);
	
	const std::vector<ConstSafePTR<Entity>>& SelectedEntities() const;

	void OnEntityRemoval(const int id);
	void Update(FactionEntities& faction_entities);

	void HandleInput(const HarvestLocationManager& harvest_locations, const Camera& camera, 
		const sf::Event& sfml_event, const sf::Window& window, const MiniMap& minimap,
		FactionHandler& faction_handler, const glm::vec3& level_size,
		const Map& map, FactionEntities& faction_entities);

	void render(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	bool Move(const glm::vec3& position, const Map& map);
	bool Repair(const glm::vec3& position, const Map& map);
	bool SetWaypoints(const glm::vec3& position, const Map& map);
	bool Harvest(const glm::vec3& destination, const Map& map, const HarvestLocationManager& baseHandler);
	bool Attack(const glm::vec3& position, FactionHandler& factionHandler, const Map& map);
	bool ReturnMinerals(const glm::vec3& position, const Map& map);
	void SelectAllOfType(const glm::vec3& position, FactionEntities& faction_entities);
	void SelectSingle(const glm::vec3& position, FactionEntities& faction_entities);
	
	const FactionPlayer* m_owning_faction			{};
	std::vector<ConstSafePTR<Entity>> m_entities	{};
	EntitySelectorBox m_selection_box{};
	glm::vec3 m_previous_mouse_position{};
	bool m_add_to_destinations_on_move{ false };
	bool m_attack_move{ false };
};