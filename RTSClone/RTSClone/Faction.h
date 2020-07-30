#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>

struct SelectionBox : private NonMovable, private NonCopyable
{
	SelectionBox();
	~SelectionBox();

	AABB AABB;
	bool active;
	glm::vec3 mouseToGroundPosition;
	glm::vec2 startingPositionScreenPosition;
	glm::vec3 startingPositionWorldPosition;
	unsigned int vaoID;
	unsigned int vboID;
};

struct Camera;
class Headquarters;
class Unit;
class Map;
class Faction : private NonMovable, private NonCopyable
{
public:
	Faction();

	void handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Unit& unit, const Map& map);
	void update(const Camera& camera, const sf::Window& window, Unit& unit, Headquarters& headquarters);
	void render(const sf::Window& window) const;

private:
	SelectionBox m_selectionBox;
};