#pragma once

#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <functional>

extern const std::array<glm::ivec2, 8> ALL_DIRECTIONS_ON_GRID;

struct AdjacentPosition
{
	AdjacentPosition();
	AdjacentPosition(glm::ivec2 position);

	bool valid;
	glm::ivec2 position;
};

using AdjacentPositionsContainer = std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>;
using AdjacentPositions = std::function<std::array<AdjacentPosition, ALL_DIRECTIONS_ON_GRID.size()>(const glm::ivec2&)>;

class AABB;
class FactionHandler;
class Worker;
class Map;
class Unit;
class EntitySpawnerBuilding;

AdjacentPositions createAdjacentPositions(const Map& map, FactionHandler& factionHandler, const Unit& unit);
AdjacentPositions createAdjacentPositions(const Map& map, const AABB& ignoreAABB);
AdjacentPositions createAdjacentPositions(const Map& map);

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const EntitySpawnerBuilding& building);

AdjacentPositionsContainer getAllAdjacentPositions(const glm::ivec2& position, const Map& map);

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map);

AdjacentPositionsContainer getRandomAdjacentPositions(const glm::ivec2& position,
	const Map& map, const Unit& unit);

AdjacentPositionsContainer getRandomAdjacentPositions(const glm::ivec2& position, const Map& map,
	const AABB& ignoreAABB);

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map,
	const AABB& ignoreAABB);

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map,
	FactionHandler& factionHandler, const Unit& unit);

AdjacentPositionsContainer getAdjacentPositions(const glm::ivec2& position, const Map& map,
	FactionHandler& factionHandler, const Unit& unit, const AABB& ignoreAABB);