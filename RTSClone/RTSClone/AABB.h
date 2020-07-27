#pragma once

#include "glm/glm.hpp"

struct AABB
{
	AABB();
	AABB(const glm::vec3& position, const glm::vec3& size);
	AABB(const glm::vec3& position, float distance);

	bool contains(const AABB& other) const;

	void reset(const glm::vec3& position, const glm::vec3& size);
	void reset(const glm::vec3& position, float distance);
	void reset();

	float m_left;
	float m_right;
	float m_forward;
	float m_back;
	float m_top;
	float m_bottom;
};