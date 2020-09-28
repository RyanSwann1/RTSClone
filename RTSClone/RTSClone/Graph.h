#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include "glm/glm.hpp"
#include <array>
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

class Graph : private NonMovable, private NonCopyable
{
public:
	Graph();

	bool isEmpty() const;
	const glm::ivec2& getPreviousPosition(const glm::ivec2& position) const;
	bool isPositionVisited(const glm::ivec2& position) const;

	void setSize(const glm::ivec2& mapSize);
	void reset(std::queue<glm::ivec2>& frontier);
	void addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition);

private:
	std::vector<GraphNode> m_graph;
	//std::array<GraphNode, static_cast<size_t>(Globals::MAP_SIZE* Globals::MAP_SIZE)> m_graph;
};