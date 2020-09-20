#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
#include "FactionHandler.h"
#include "EntityTarget.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Map;
struct Camera;
class ShaderHandler;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	~Level();

	eFactionController getWinningFactionController() const;
	bool isComplete() const;

	void handleEvent(const GameEvent& gameEvent, const Map& map);
	void handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent, const Map& map);
	void update(float deltaTime, const Map& map);
	void renderSelectionBox(const sf::Window& window) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void render(ShaderHandler& shaderHandler) const;
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Level(std::vector<SceneryGameObject>&& scenery, 
		std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1>&& factions);
	
	std::vector<SceneryGameObject> m_scenery;
	std::array<std::unique_ptr<Faction>, static_cast<size_t>(eFactionController::Max) + 1> m_factions;
	FactionHandler m_factionHandler;
	ProjectileHandler m_projectileHandler;
	EntityTarget m_selectedTarget;

	void setAITargetFaction();
	void handleGUI();
};	