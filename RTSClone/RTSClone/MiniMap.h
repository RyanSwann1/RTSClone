#pragma once

#include "Sprite.h"
#include "OpenGLResource.h"
#include <SFML/Graphics.hpp>

class Camera;
class ShaderHandler;
class Level;
class MiniMap
{
public:
	MiniMap(glm::ivec2 position, glm::ivec2 size);
	MiniMap(const MiniMap&) = delete;
	MiniMap& operator=(const MiniMap&) = delete;
	MiniMap(MiniMap&&) = delete;
	MiniMap& operator=(MiniMap&&) = delete;

	bool isMouseButtonPressed() const;

	bool handleInput(glm::uvec2 windowSize, const sf::Window& window, const glm::vec3& levelSize, Camera& camera,
		sf::Event sfmlEvent);
	void render(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const Level& level, const Camera& camera) const;

private:
	const Sprite m_background;
	const glm::ivec2 m_position;
	const glm::ivec2 m_size;
	Sprite m_entitySprite;
	bool m_mouseButtonPressed;
};