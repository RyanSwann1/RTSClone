#pragma once

#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <functional>

constexpr std::array<glm::ivec2, 8> ALL_DIRECTIONS_ON_GRID =
{
	glm::ivec2(0, 1),
	glm::ivec2(1, 1),
	glm::ivec2(1, 0),
	glm::ivec2(1, -1),
	glm::ivec2(0, -1),
	glm::ivec2(-1, -1),
	glm::ivec2(-1, 0),
	glm::ivec2(-1, 1)
};

struct AdjacentPosition
{
	AdjacentPosition();
	AdjacentPosition(const glm::ivec2& position);
	AdjacentPosition(const glm::ivec2& position, bool valid, bool unitCollision);

	bool valid;
	bool unitCollision;
	glm::ivec2 position;
};

using GetAllAdjacentPositions = const std::function<std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>(const glm::ivec2&)>&;

class Map;
class Unit;
std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::vector<Unit>& units);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::vector<Unit>& units, const Unit& unit);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::vector<Unit>& units, const Unit& unit, const std::vector<const Unit*>& selectedUnits);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map);