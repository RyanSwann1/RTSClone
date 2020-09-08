#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameObject.h"
#include "ProjectileHandler.h"
#include "Map.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

#define RENDER_GROUND
#ifdef RENDER_GROUND
#include "Ground.h"
#endif // RENDER_GROUND

struct Camera;
class ShaderHandler;
class Level : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);
	
	void handleInput(const sf::Window& window, const Camera& camera, const sf::Event& currentSFMLEvent);
	void update(float deltaTime);
	void renderSelectionBox(const sf::Window& window) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

#ifdef RENDER_GROUND
	void renderGround(ShaderHandler& shaderHandler) const;
#endif // RENDER_GROUND

private:
	Level(std::vector<GameObject>&& scenery);
	const std::vector<GameObject> m_scenery;
	Map m_map;
	ProjectileHandler m_projectileHandler;
	FactionPlayer m_player;
	FactionAI m_playerAI;
#ifdef RENDER_GROUND
	Ground m_ground;
#endif // RENDER_GROUND
};