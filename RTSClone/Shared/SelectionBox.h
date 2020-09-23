#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>

class SelectionBox : private NonCopyable, private NonMovable
{
public:
	SelectionBox();
	~SelectionBox();

	const AABB& getAABB() const;
	bool isActive() const;
	bool isMinimumSize() const;

	void setStartingPosition(const sf::Window& window, const glm::vec3& mouseToGroundPosition);
	void setSize(const glm::vec3& mouseToGroundPosition);
	void reset();
	void render(const sf::Window& window) const;

private:
	AABB m_AABB;
	bool m_active;
	glm::vec2 m_screenStartingPosition;
	glm::vec3 m_worldStartingPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;
};