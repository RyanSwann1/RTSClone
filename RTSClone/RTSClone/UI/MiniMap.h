#pragma once

#include "UI/Sprite.h"
#include "Graphics/OpenGLResource.h"
#include <SFML/Graphics.hpp>

struct Camera;
class ShaderHandler;
class Level;
class MiniMap
{
public:
	MiniMap();
	MiniMap(const MiniMap&) = delete;
	MiniMap& operator=(const MiniMap&) = delete;
	MiniMap(MiniMap&&) noexcept = default;
	MiniMap& operator=(MiniMap&&) noexcept = default;

	glm::vec3 get_relative_intersecting_position(const sf::Window& window, const glm::vec3& levelSize) const;
	glm::vec2 getPosition() const;
	glm::vec2 getSize() const;
	bool isIntersecting(const sf::Window& window) const;
	bool isUserInteracted() const;

	bool handleInput(glm::uvec2 windowSize, const sf::Window& window, const glm::vec3& levelSize, Camera& camera,
		sf::Event sfmlEvent);
	void render(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const Level& level, const Camera& camera,
		const sf::Window& window) const;

private:
	Sprite m_background;
	glm::vec2 m_position;
	glm::vec2 m_size;
	Sprite m_entitySprite;
	Sprite m_cameraViewSprite;
	bool m_mouseButtonPressed;
};