#pragma once

#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <functional>

extern const std::array<glm::ivec2, 8> ALL_DIRECTIONS_ON_GRID;

struct AdjacentPosition
{
	AdjacentPosition();
	AdjacentPosition(const glm::ivec2& position);
	AdjacentPosition(const glm::ivec2& position, bool valid);

	bool valid;
	bool used;
	glm::ivec2 position;
};

using AdjacentPositionsContainer = std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>;
using AdjacentPositions = std::function<std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>(const glm::ivec2&)>;

class AABB;
class FactionHandler;
class Worker;
class Map;
class Unit;

AdjacentPositions createAdjacentPositions(const Map& map, FactionHandler& factionHandler, const Unit& unit);
AdjacentPositions createAdjacentPositions(const Map& map, const AABB& ignoreAABB);
AdjacentPositions createAdjacentPositions(const Map& map);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const std::vector<std::unique_ptr<Unit>>& units, const std::vector<std::unique_ptr<Worker>>& workers, const AABB& ignoreAABB);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAllAdjacentPositions(const glm::ivec2& position, const Map& map);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getRandomAdjacentPositions(const glm::ivec2& position, const Map& map, 
	const AABB& ignoreAABB);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const AABB& ignoreAABB);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	FactionHandler& factionHandler, const Unit& unit);

std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()> getAdjacentPositions(const glm::ivec2& position, const Map& map,
	FactionHandler& factionHandler, const Unit& unit, const AABB& ignoreAABB);