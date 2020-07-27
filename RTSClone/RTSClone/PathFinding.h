#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "Globals.h"
#include <vector>
#include <queue>
#include <array>

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

class Graph
{
public:
	Graph();

	bool isEmpty() const;
	const glm::ivec2& getPreviousPosition(const glm::ivec2& position) const;
	bool isPositionVisited(const glm::ivec2& position) const;

	void resetGraph();
	void addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition);

private:
	std::array<GraphNode, static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)> m_graph;
};

class PathFinding
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	void getPathToPosition(const glm::vec3& startingPosition, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition);

private:
	PathFinding();

	Graph m_graph;
	std::queue<glm::ivec2> m_frontier;
};