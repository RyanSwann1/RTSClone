#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>

struct SelectionBox : private NonCopyable, private NonMovable
{
	SelectionBox();
	~SelectionBox();

	bool isMinimumSize() const;

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void setSize(const glm::vec3& position);
	void reset();
	void render(const sf::Window& window) const;

	AABB AABB;
	bool active;
	glm::vec3 mouseToGroundPosition;
	glm::vec2 startingPositionScreenPosition;
	glm::vec3 startingPositionWorldPosition;

private:
	unsigned int vaoID;
	unsigned int vboID;
};