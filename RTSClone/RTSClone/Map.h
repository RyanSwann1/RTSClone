#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include <array>

class Building;
class Map : private NonMovable, private NonCopyable
{
public:
	Map();

	bool isPositionOccupied(const glm::ivec2& position) const;

	void addBuilding(const Building& building);

private:
	std::array<bool, static_cast<size_t>(Globals::MAP_SIZE* Globals::MAP_SIZE)> m_map;
};