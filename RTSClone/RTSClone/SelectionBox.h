#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include <SFML/Graphics.hpp>

class SelectionBox : private NonMovable, private NonCopyable
{
public:
	SelectionBox();
	~SelectionBox();

	void handleInputEvents(const sf::Event& currentSFMLEvent, const sf::Window& window);
	void render(const sf::Window& window) const;

private:
	bool m_active;
	glm::ivec2 m_startingPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;
};