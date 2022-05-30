#pragma once

#include <glm/glm.hpp>

enum class GridLockActive
{
	True,
	False
};

class Position
{
public:
	Position(const glm::vec3& position, const GridLockActive grid_locked);

	static glm::vec3 CreateGridLocked(const glm::vec3& position);

	const glm::vec3& Get() const;
	const glm::vec3& Set(const glm::vec3& position);

private:
	GridLockActive m_grid_locked{ GridLockActive::False };
	glm::vec3 m_position{};
};