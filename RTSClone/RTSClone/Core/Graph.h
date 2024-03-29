#pragma once

#include "Core/Globals.h"
#include "glm/glm.hpp"
#include "Events/GameMessenger.h"
#include <array>
#include <vector>
#include <queue>
#include <optional>

namespace GameMessages
{
	struct MapSize;
}
class Map;
class Graph 
{
public:
	Graph();

	bool is_frontier_empty() const;
	glm::ivec2 pop_frontier();
	bool is_position_visited(glm::ivec2 position, const Map& map) const;

	void reset(glm::ivec2 startingPosition);
	void add(glm::ivec2 position, glm::ivec2 cameFromPosition, const Map& map);

private:
	std::queue<glm::ivec2> m_frontier = {};
	glm::ivec2 m_size = { 0, 0 };
	std::vector<std::optional<glm::ivec2>> m_graph = {};
	BroadcasterSub<GameMessages::MapSize> m_onNewMapSizeID;
	
	void new_map_size(const GameMessages::MapSize& gameMessage);
};