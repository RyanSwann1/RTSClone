#include "Rectangle2D.h"

Rectangle2D::Rectangle2D()
	: m_left(0.0f),
	m_right(0.0f),
	m_top(0.0f),
	m_bottom(0.0f)
{}

Rectangle2D::Rectangle2D(const glm::vec2& position, const glm::vec2& size)
	: m_left(glm::min(position.x, position.x + size.x)),
	m_right(glm::max(position.x, position.x + size.x)),
	m_top(glm::max(position.y, position.y + size.y)),
	m_bottom(glm::min(position.y, position.y + size.y))
{}

Rectangle2D::Rectangle2D(const glm::vec3 & position, float distance)
	: m_left(position.x - distance),
	m_right(position.x + distance),
	m_top(position.z + distance),
	m_bottom(position.z - distance)
{}

bool Rectangle2D::contains(const glm::vec2& position) const
{
	return position.x >= m_left &&
		position.x <= m_right &&
		position.y <= m_top &&
		position.y >= m_bottom;
}

bool Rectangle2D::contains(const Rectangle2D& other) const
{
	return m_left <= other.m_right &&
		m_right >= other.m_left &&
		m_top >= other.m_bottom &&
		m_bottom <= other.m_top;
}

void Rectangle2D::reset(const glm::vec2& position, const glm::vec2& size)
{
	m_left = glm::min(position.x, position.x + size.x);
	m_right = glm::max(position.x, position.x + size.x);
	m_top = glm::max(position.y, position.y + size.y);
	m_bottom = glm::min(position.y, position.y + size.y);
}

void Rectangle2D::reset()
{
	m_left = 0.0f;
	m_right = 0.0f;
	m_top = 0.0f;
	m_bottom = 0.0f;
}