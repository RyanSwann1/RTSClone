#pragma once

#include "glm/glm.hpp"

//https://silentmatt.com/rectangle-intersection/
class Rectangle2D
{
public:
	Rectangle2D();
	Rectangle2D(const glm::vec2& position, const glm::vec2& size);
	Rectangle2D(const glm::vec3& position, float distance);

	bool contains(const glm::vec2& position) const;
	bool contains(const Rectangle2D& other) const;

	void reset(const glm::vec2& position, const glm::vec2& size);
	void reset();

private:
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;
};