#include "AABB.h"

AABB::AABB()
	: m_left(0.0f),
	m_right(0.0f),
	m_forward(0.0f),
	m_back(0.0f),
	m_top(0.0f),
	m_bottom(0.0f)
{}

AABB::AABB(const glm::vec3& position, const glm::vec3& size)
	: m_left(glm::min(position.x, position.x + size.x)),
	m_right(glm::max(position.x, position.x + size.x)),
	m_top(glm::max(position.y, position.y + size.y)),
	m_bottom(glm::min(position.y, position.y + size.y)),
	m_forward(glm::max(position.z, position.z + size.z)),
	m_back(glm::min(position.z, position.z + size.z))
{}

AABB::AABB(const glm::vec3& position, float distance)
	: m_left(position.x - distance),
	m_right(position.x + distance),
	m_top(position.y + distance),
	m_bottom(position.y - distance),
	m_forward(position.z + distance),
	m_back(position.z - distance)
{ }

bool AABB::contains(const AABB& other) const
{
	return m_left <= other.m_right &&
		m_right >= other.m_left &&
		m_top >= other.m_bottom &&
		m_bottom <= other.m_top &&
		m_forward >= other.m_back &&
		m_back <= other.m_forward;
}

void AABB::reset(const glm::vec3& position, const glm::vec3& size)
{
	m_left = glm::min(position.x, position.x + size.x);
	m_right = glm::max(position.x, position.x + size.x);
	m_top = glm::max(position.y, position.y + size.y);
	m_bottom = glm::min(position.y, position.y + size.y);
	m_forward = glm::max(position.z, position.z + size.z);
	m_back = glm::min(position.z, position.z + size.z);
}

void AABB::reset(const glm::vec3& position, float distance)
{
	m_left = position.x - distance;
	m_right = position.x + distance;
	m_top = position.y + distance;
	m_bottom = position.y - distance;
	m_forward = position.z + distance;
	m_back = position.z - distance;
}

void AABB::reset()
{
	m_left = 0.0f;
	m_right = 0.0f;
	m_top = 0.0f;
	m_bottom = 0.0f;
	m_forward = 0.0f;
	m_back = 0.0f;
}