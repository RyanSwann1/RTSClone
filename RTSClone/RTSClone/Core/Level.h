#pragma once

#include "Model/Projectile.h"
#include "Factions/FactionPlayer.h"
#include "Factions/FactionAI.h"
#include "Scene/SceneryGameObject.h"
#include "Factions/FactionHandler.h"
#include "Core/Base.h"
#include "Graphics/Quad.h"
#include "UI/MiniMap.h"
#include "Core/Camera.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <optional>
#include <chrono>

struct LevelDetailsFromFile
{
	std::vector<SceneryGameObject> scenery	= {};
	std::vector<HarvestLocation> harvest_locations = {};
	int factionStartingResources = 0;
	int factionStartingPopulation = 0;
	int factionCount = 0;
	glm::vec3 size = {};
	glm::ivec2 gridSize = {};
};

class UIManager;
class ShaderHandler;
class Level
{
public:
	Level(LevelDetailsFromFile&& levelDetails, glm::ivec2 windowSize);
	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;
	Level(Level&&) noexcept = default;
	Level& operator=(Level&&) noexcept = default;
	~Level();

	static std::optional<LevelDetailsFromFile> load(std::string_view levelName, glm::ivec2 windowSize);
	static void add_event(const GameEvent& gameEvent);

	const std::vector<SceneryGameObject>& getSceneryGameObjects() const;
	const HarvestLocationManager GetHarvestLocationManager() const { return m_harvest_location_manager; }
	const Camera& getCamera() const;
	bool isMinimapInteracted() const;
	const std::vector<std::unique_ptr<Faction>>& getFactions() const;
	const glm::vec3& getSize() const;
	const Faction* getWinningFaction() const;

	void handleInput(glm::uvec2 windowSize, const sf::Window& window, const sf::Event& currentSFMLEvent, UIManager& uiManager);
	void update(float deltaTime, UIManager& uiManager, glm::uvec2 windowSize, const sf::Window& window);
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const;
	void renderTerrain(ShaderHandler& shaderHandler) const;
	void renderPlayerPlannedBuilding(ShaderHandler& shaderHandler) const;
	void renderMinimap(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const sf::Window& window) const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	HarvestLocationManager m_harvest_location_manager;
	std::vector<SceneryGameObject> m_scenery;
	Map m_map;
	std::vector<Projectile> m_projectiles;
	Quad m_playableArea;
	Camera m_camera;
	MiniMap m_minimap;
	std::chrono::high_resolution_clock::time_point m_lastDelayedUpdate = std::chrono::high_resolution_clock::now();
	FactionHandler m_factionHandler;

	void handleEvent(const GameEvent& gameEvent, const Map& map);
};	