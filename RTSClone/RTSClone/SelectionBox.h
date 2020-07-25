#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct Unit;
struct Camera;
class SelectionBox : private NonMovable, private NonCopyable
{
public:
	SelectionBox();
	~SelectionBox();

	void update(const glm::mat4& projection, const glm::mat4& view, const Camera& camera, const sf::Window& window,
		Unit& unit);
	void handleInputEvents(const sf::Event& currentSFMLEvent, const sf::Window& window, const glm::mat4& projection,
		const glm::mat4& view, const Camera& camera);
	void render(const sf::Window& window) const;

private:
	AABB m_selectionBox;
	bool m_active;
	glm::vec2 m_startingPositionScreenPosition;
	glm::vec3 m_startingPositionWorldPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;
};