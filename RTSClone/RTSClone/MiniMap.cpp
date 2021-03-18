#include "MiniMap.h"
#include "Camera.h"
#include <iostream>

namespace
{
	const glm::vec3 BACKGROUND_COLOR = { 0.0f, 0.0f, 0.0f };

	bool isWithinBounds(glm::ivec2 mousePosition, glm::ivec2 position, glm::ivec2 size)
	{
		return mousePosition.x >= position.x &&
			mousePosition.x <= position.x + size.x &&
			mousePosition.y >= position.y &&
			mousePosition.y <= position.y + size.y;
	}
}

MiniMap::MiniMap(glm::ivec2 position, glm::ivec2 size)
	: m_background(),
	m_position(position),
	m_size(size)
{}

void MiniMap::handleInput(glm::uvec2 windowSize, const sf::Window& window, const glm::vec3& levelSize, Camera& camera)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		glm::ivec2 mousePosition(sf::Mouse::getPosition(window).x, windowSize.y - sf::Mouse::getPosition(window).y);
		if (isWithinBounds(mousePosition, m_position, m_size))
		{
			glm::vec2 convertedMousePosition(static_cast<float>(mousePosition.x - m_position.x) / m_size.x, 
				static_cast<float>(mousePosition.y - m_position.y) / m_size.y);
			camera.setPosition({ convertedMousePosition.x * levelSize.x, convertedMousePosition.y * levelSize.z });
		}
	}
}

void MiniMap::render(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const
{
	m_background.render(m_position, m_size, BACKGROUND_COLOR, shaderHandler, windowSize, 0.25f);
}