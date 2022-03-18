#pragma once

#include "Projectile.h"
#include "Factions/FactionPlayer.h"
#include "Factions/FactionAI.h"
#include "SceneryGameObject.h"
#include "Factions/FactionHandler.h"
#include "Timer.h"
#include "Base.h"
#include "Quad.h"
#include "MiniMap.h"
#include "Camera.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <optional>

struct LevelDetailsFromFile
{
	std::vector<SceneryGameObject> scenery	= {};
	std::vector<Base> bases					= {};
	int factionStartingResources			= 0;
	int factionStartingPopulation			= 0;
	int factionCount						= 0;
	glm::vec3 size							= {};
};

class UIManager;
class ShaderHandler;
class Level
{
public:
	Level(LevelDetailsFromFile&& levelDetails, glm::ivec2 windowSize);
	Level& operator=(Level&&) noexcept = default;
	Level(Level&&) noexcept = default;
	~Level();

	static std::optional<Level> load(std::string_view levelName, glm::ivec2 windowSize);
	static void add_event(const GameEvent& gameEvent);

	const std::vector<SceneryGameObject>& getSceneryGameObjects() const;
	const BaseHandler& getBaseHandler() const;
	const Camera& getCamera() const;
	bool isMinimapInteracted() const;
	const FactionsContainer& getFactions() const;
	const glm::vec3& getSize() const;
	const Faction* getWinningFaction() const;

	void handleInput(glm::uvec2 windowSize, const sf::Window& window, const sf::Event& currentSFMLEvent, const Map& map,
		UIManager& uiManager);
	void update(float deltaTime, const Map& map, UIManager& uiManager, glm::uvec2 windowSize, const sf::Window& window);
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const;
	void renderTerrain(ShaderHandler& shaderHandler) const;
	void renderPlayerPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const;
	void renderBasePositions(ShaderHandler& shaderHandler) const;
	void renderMinimap(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const sf::Window& window) const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	std::vector<Projectile> m_projectiles;
	Quad m_playableArea;
	BaseHandler m_baseHandler;
	std::vector<SceneryGameObject> m_scenery;
	Camera m_camera;
	MiniMap m_minimap;
	Timer m_unitStateHandlerTimer;
	FactionHandler m_factionHandler;

	void handleEvent(const GameEvent& gameEvent, const Map& map);
};	