#include "MiniMap.h"
#include "Camera.h"
#include "Level.h"
#include <iostream>

namespace
{
	const glm::ivec2 STARTING_POSITION = { 50, 50 };
	const glm::ivec2 STARTING_SIZE = { 250, 250 };
	const glm::vec3 BACKGROUND_COLOR = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 FRIENDLY_ENTITY_COLOR = { 0.0f, 1.0f, 0.0f };
	const glm::vec3 UNFRIENDLY_ENTITY_COLOR = { 1.0f, 0.0f, 0.0f };

	bool isWithinBounds(glm::ivec2 mousePosition, glm::ivec2 position, glm::ivec2 size)
	{
		return mousePosition.x >= position.x &&
			mousePosition.x <= position.x + size.x &&
			mousePosition.y >= position.y &&
			mousePosition.y <= position.y + size.y;
	}
}

MiniMap::MiniMap()
	: m_background(),
	m_position(STARTING_POSITION),
	m_size(STARTING_SIZE),
	m_entitySprite(),
	m_mouseButtonPressed(false)
{}

bool MiniMap::isMouseButtonPressed() const
{
	return m_mouseButtonPressed;
}

bool MiniMap::handleInput(glm::uvec2 windowSize, const sf::Window& window, const glm::vec3& levelSize, Camera& camera,
	sf::Event sfmlEvent)
{
	if (sfmlEvent.type == sf::Event::MouseButtonReleased && sfmlEvent.mouseButton.button == sf::Mouse::Button::Left)
	{
		m_mouseButtonPressed = false;
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		glm::ivec2 mousePosition(sf::Mouse::getPosition(window).x, windowSize.y - sf::Mouse::getPosition(window).y);
		if (isWithinBounds(mousePosition, m_position, m_size))
		{
			m_mouseButtonPressed = true;
			glm::vec2 convertedMousePosition(static_cast<float>(mousePosition.x - m_position.x) / m_size.x, 
				static_cast<float>(mousePosition.y - m_position.y) / m_size.y);
			camera.setPosition({ convertedMousePosition.x * levelSize.x, convertedMousePosition.y * levelSize.z });
		
			return true;
		}
	}

	return false;
}

void MiniMap::render(ShaderHandler& shaderHandler, glm::uvec2 windowSize, const Level& level, const Camera& camera) const
{
	m_background.render(m_position, m_size, BACKGROUND_COLOR, shaderHandler, windowSize, 0.8f);

	for (const auto& faction : level.getFactions())
	{
		if (!faction)
		{
			continue;
		}

		glm::vec3 color(0.0f);
		switch (faction.get()->getController())
		{
		case eFactionController::Player:
			color = FRIENDLY_ENTITY_COLOR;
			break;
		case eFactionController::AI_1:
		case eFactionController::AI_2:
		case eFactionController::AI_3:
			color = UNFRIENDLY_ENTITY_COLOR;
			break;
		default:
			assert(false);
		}
		for (const auto& entity : faction.get()->getAllEntities())
		{
			const glm::vec3& entityPosition = entity.get().getPosition();
			glm::vec2 convertedEntityPosition((entityPosition.z / level.getSize().x) * m_size.x, (entityPosition.x / level.getSize().z) * m_size.y);
			convertedEntityPosition += m_position;

			m_entitySprite.render(convertedEntityPosition, { 3, 3 }, color, shaderHandler, windowSize, 0.75f);
		}
	}
}