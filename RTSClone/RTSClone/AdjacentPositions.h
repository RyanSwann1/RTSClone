#pragma once

#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <functional>
#include <forward_list>

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
	AdjacentPosition(const glm::ivec2& position, bool valid);

	bool valid;
	glm::ivec2 position;
};

using AdjacentPositions = const std::function<std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>(const glm::ivec2&)>&;

class FactionPlayer;
class FactionHandler;
class Worker;
class Map;
class Unit;
std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::forward_list<Unit>& units);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::forward_list<Unit>& units, const Unit& unit);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::forward_list<Unit>& units, const Unit& unit, const std::vector<Unit*>& selectedUnits);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	FactionHandler& factionHandler, const Unit& unit);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const FactionPlayer& factionPlayer, const Unit& unit);