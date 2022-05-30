#include "Position.h"
#include "Core/Globals.h"

namespace
{
	glm::vec3 GetGridLockedPosition(glm::vec3 position)
	{
		position = Globals::convertToNodePosition(position);
		position = Globals::convertToMiddleGridPosition(position);
		
		return position;
	}
}

Position::Position(const glm::vec3& position, const GridLockActive grid_locked)
	: m_grid_locked(grid_locked),
	m_position(Set(position))
{}

glm::vec3 Position::CreateGridLocked(const glm::vec3& position)
{
	return GetGridLockedPosition(position);
}

const glm::vec3& Position::Get() const
{
	return m_position;
}

const glm::vec3& Position::Set(const glm::vec3& position)
{
	if (m_grid_locked == GridLockActive::True)
	{
		m_position = GetGridLockedPosition(position);
		return m_position;
	}

	m_position = position;
	return m_position;
}