#pragma once

#include <glm/glm.hpp>

class Position
{
public:
	Position(const glm::vec3& position, const bool grid_locked);

	static glm::vec3 CreateGridLocked(const glm::vec3& position);

	const glm::vec3& Get() const;
	const glm::vec3& Set(const glm::vec3& position);

private:
	bool m_grid_locked{ false };
	glm::vec3 m_position{};
};