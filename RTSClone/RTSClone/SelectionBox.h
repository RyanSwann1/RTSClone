#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>
#include <vector>

class Headquarters;
class Unit;
class Map;
struct Camera;
class SelectionBox : private NonMovable, private NonCopyable
{
public:
	SelectionBox();
	~SelectionBox();

	void update(const Camera& camera, const sf::Window& window, Unit& unit, Headquarters& building);
	void handleInputEvents(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Unit& unit, const Map& map);
	void render(const sf::Window& window) const;

private:
	AABB m_selectionBox;
	bool m_active;
	glm::vec3 m_mouseToGroundPosition;
	glm::vec2 m_startingPositionScreenPosition;
	glm::vec3 m_startingPositionWorldPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;
};