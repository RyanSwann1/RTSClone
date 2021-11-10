#pragma once

#include "Globals.h"
#include "glm/glm.hpp"
#include "GameMessengerSubscriber.h"
#include <array>
#include <vector>
#include <queue>

struct GraphNode
{
	GraphNode();
	GraphNode(const glm::ivec2& cameFrom);

	glm::ivec2 cameFrom;
	bool visited;
};

namespace GameMessages
{
	struct NewMapSize;
}
class Map;
class Graph 
{
public:
	Graph();
	Graph(const Graph&) = delete;
	Graph& operator=(const Graph&) = delete;
	Graph(Graph&&) = default;
	Graph& operator=(Graph&&) = default;

	bool isEmpty() const;
	const glm::ivec2& getPreviousPosition(const glm::ivec2& position, const Map& map) const;
	bool isPositionVisited(const glm::ivec2& position, const Map& map) const;

	void reset(std::queue<glm::ivec2>& frontier);
	void addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition, const Map& map);

private:
	glm::ivec2 m_size;
	std::vector<GraphNode> m_graph;
	GameMessengerSubscriber<GameMessages::NewMapSize> m_onNewMapSizeID;
	
	void onNewMapSize(const GameMessages::NewMapSize& gameMessage);
	void reset();
};