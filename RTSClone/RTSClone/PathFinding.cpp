#include "PathFinding.h"
#include "Globals.h"
#include <array>

namespace
{
	bool isPositionInMapBounds(const glm::ivec2& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE;
	}

	bool isPositionWithinNodeSize(const glm::ivec2& nodePosition, const glm::ivec2& position)
	{
		return position.x >= nodePosition.x &&
			position.x <= nodePosition.x + Globals::NODE_SIZE &&
			position.y >= nodePosition.y &&
			position.y <= nodePosition.y + Globals::NODE_SIZE;
	}

	int convertTo1D(const glm::ivec2& position)
	{
		return position.x * Globals::MAP_SIZE + position.y;
	}

	glm::vec3 convertToWorldPosition(const glm::ivec2& position)
	{
		return { position.x * Globals::NODE_SIZE, Globals::GROUND_HEIGHT, position.y * Globals::NODE_SIZE };
	}

	glm::ivec2 convertToGridPosition(const glm::vec3& position)
	{
		return { position.x / Globals::NODE_SIZE, position.z / Globals::NODE_SIZE };
	}

	std::array<glm::ivec2, 8> ALL_DIRECTIONS =
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

	std::vector<glm::ivec2> getAllAdjacentPositions(const glm::ivec2& position, const glm::ivec2& ignorePosition)
	{
		std::vector<glm::ivec2> adjacentPositions;
		for (const auto& direction : ALL_DIRECTIONS)
		{
			glm::ivec2 newPosition = position + direction;
			if (newPosition != ignorePosition && isPositionInMapBounds(newPosition))
			{
				adjacentPositions.push_back(newPosition);
			}
		}

		return adjacentPositions;
	}

	void getPathFromVisitedNodes(const glm::ivec2& startingPosition, const glm::ivec2& destination, 
		std::vector<glm::vec3>& pathToPosition, Graph& graph)
	{
		pathToPosition.push_back(convertToWorldPosition(destination));
		glm::ivec2 position = destination;

		while (position != startingPosition)
		{
			pathToPosition.push_back(convertToWorldPosition(position));
			position = graph.getPreviousPosition(position);
		}
	}
}

//Graph Node
GraphNode::GraphNode()
	: cameFrom(),
	visited(false)
{}

GraphNode::GraphNode(const glm::ivec2& cameFrom)
	: cameFrom(cameFrom),
	visited(true)
{}

const glm::ivec2& GraphNode::getCameFrom() const
{
	return cameFrom;
}

bool GraphNode::isVisited() const
{
	return visited;
}

//Graph
Graph::Graph()
	: m_graph()
{}

void Graph::addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition)
{
	assert(isPositionInMapBounds(position) && !m_graph[convertTo1D(position)].isVisited());
	if (isPositionInMapBounds(position) && !m_graph[convertTo1D(position)].isVisited())
	{
		m_graph[convertTo1D(position)] = GraphNode(cameFromPosition);
	}
}

bool Graph::isEmpty() const
{
	return m_graph.empty();
}

const glm::ivec2& Graph::getPreviousPosition(const glm::ivec2& position) const
{
	assert(isPositionInMapBounds(position) && m_graph[convertTo1D(position)].isVisited());
	if (isPositionInMapBounds(position) && m_graph[convertTo1D(position)].isVisited())
	{
		return m_graph[convertTo1D(position)].getCameFrom();
	}
}

bool Graph::isPositionVisited(const glm::ivec2& position) const
{
	assert(isPositionInMapBounds(position));
	if (isPositionInMapBounds(position))
	{
		return m_graph[convertTo1D(position)].isVisited();
	}
}

void Graph::resetGraph()
{
	for (auto& i : m_graph)
	{
		i = GraphNode();
	}
}

//PathFinding
PathFinding::PathFinding()
	: m_graph(),
	m_frontier()
{}

void PathFinding::getPathToPosition(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition)
{
	m_graph.resetGraph();
	
	glm::ivec2 destinationPositionOnGrid = convertToGridPosition(destinationPosition);
	glm::ivec2 startingPositionOnGrid = convertToGridPosition(startingPosition);
	m_frontier.push(startingPositionOnGrid);
	bool destinationReached = false;

	while (!m_frontier.empty() && !destinationReached)
	{
		glm::ivec2 position = m_frontier.front();
		m_frontier.pop();

		std::vector<glm::ivec2> adjacentPositions = getAllAdjacentPositions(position, startingPositionOnGrid);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPosition))
			{
				m_graph.addToGraph(adjacentPosition, position);
				m_frontier.push(adjacentPosition);
			}
	
			if (adjacentPosition == destinationPositionOnGrid)
			{
				destinationReached = true;
				getPathFromVisitedNodes(startingPositionOnGrid, adjacentPosition, pathToPosition, m_graph);
				break;
			}
		}

		assert(m_frontier.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE + 50);
	}

	assert(!pathToPosition.empty());
}