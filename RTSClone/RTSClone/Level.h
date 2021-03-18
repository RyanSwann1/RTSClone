#pragma once

#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#include "FactionHandler.h"
#include "Timer.h"
#include "Base.h"
#include "Quad.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

using FactionsContainer = std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>;

struct Camera;
class UIManager;
class ShaderHandler;
class Level
{
public:
	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;
	Level(Level&&) = delete;
	Level& operator=(Level&&) = delete;
	~Level();

	static std::unique_ptr<Level> create(const std::string& levelName, Camera& camera);

	const glm::vec3& getSize() const;
	const Faction* getWinningFaction() const;

	void handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map,
		UIManager& uiManager);
	void update(float deltaTime, const Map& map, UIManager& uiManager);
	void renderEntitySelector(const sf::Window& window, ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void renderTerrain(ShaderHandler& shaderHandler) const;
	void renderPlayerPlannedBuilding(ShaderHandler& shaderHandler, const Map& map) const;
	void renderBasePositions(ShaderHandler& shaderHandler) const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Level(std::vector<SceneryGameObject>&& scenery, FactionsContainer&& factions, 
		std::unique_ptr<BaseHandler>&& baseHandler, const glm::vec3& size);

	const Quad m_playableArea;
	const std::unique_ptr<BaseHandler> m_baseHandler;
	const std::vector<SceneryGameObject> m_scenery;
	FactionsContainer m_factions;
	Timer m_unitStateHandlerTimer;
	FactionHandler m_factionHandler;
	ProjectileHandler m_projectileHandler;

	void handleEvent(const GameEvent& gameEvent, const Map& map);
};	