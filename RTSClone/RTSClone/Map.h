#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include <array>

struct AABB;
class Map : private NonMovable, private NonCopyable
{
public:
	Map();

	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;

	void addEntityAABB(const AABB& AABB);

private:
	std::array<bool, static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)> m_map;
};