#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include "glm/glm.hpp"
#include <array>
#include <vector>
#include <queue>

class GraphNode
{
public:
	GraphNode();
	GraphNode(const glm::ivec2& cameFrom);

	const glm::ivec2& getCameFrom() const;
	bool isVisited() const;

private:
	glm::ivec2 cameFrom;
	bool visited;
};

namespace GameMessages
{
	struct NewMapSize;
}
class Map;
class Graph : private NonMovable, private NonCopyable
{
public:
	Graph();
	~Graph();

	bool isEmpty() const;
	const glm::ivec2& getPreviousPosition(const glm::ivec2& position, const Map& map) const;
	bool isPositionVisited(const glm::ivec2& position, const Map& map) const;

	void reset(std::queue<glm::ivec2>& frontier);
	void addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition, const Map& map);

private:
	glm::ivec2 m_size;
	std::vector<GraphNode> m_graph;

	void onNewMapSize(const GameMessages::NewMapSize& gameMessage);
	void reset();
};