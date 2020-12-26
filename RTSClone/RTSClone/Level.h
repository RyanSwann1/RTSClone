#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#include "FactionHandler.h"
#include "Timer.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

using FactionsContainer = std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>;

struct Camera;
class UIManager;
class ShaderHandler;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	~Level();

	const Faction* getPlayer() const;
	const Faction* getWinningFaction() const;

	void handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map,
		UIManager& uiManager);
	void update(float deltaTime, const Map& map, UIManager& uiManager);
	void renderSelectionBox(const sf::Window& window) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Level(std::vector<SceneryGameObject>&& scenery, FactionsContainer&& factions);

	std::vector<SceneryGameObject> m_scenery;
	FactionsContainer m_factions;
	Timer m_unitStateHandlerTimer;
	FactionHandler m_factionHandler;
	ProjectileHandler m_projectileHandler;

	void setAITargetFaction();
	void handleEvent(const GameEvent& gameEvent, const Map& map);
};	