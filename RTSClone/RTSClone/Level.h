#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "SceneryGameObject.h"
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
	Level(std::vector<SceneryGameObject>&& scenery, std::vector<std::unique_ptr<Faction>>&& factions);
	std::vector<SceneryGameObject> m_scenery;
	
	ProjectileHandler m_projectileHandler;
	std::vector<std::unique_ptr<Faction>> m_factions;
	FactionPlayer* m_player;
	FactionAI* m_playerAI;
};	