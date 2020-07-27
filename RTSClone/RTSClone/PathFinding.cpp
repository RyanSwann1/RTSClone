#include "PathFinding.h"
#include "Globals.h"
#include <array>

namespace
{
	struct AdjacentPosition
	{
		AdjacentPosition()
			: valid(false),
			position()
		{}
		AdjacentPosition(bool valid)
			: valid(valid),
			position()
		{}
		AdjacentPosition(const glm::ivec2& position)
			: valid(true),
			position(position)
		{}

		bool valid;
		glm::ivec2 position;
	};

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

	constexpr std::array<glm::ivec2, 8> ALL_DIRECTIONS =
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

	std::array<AdjacentPosition, ALL_DIRECTIONS.size()> getAllAdjacentPositions(const glm::ivec2& position, const glm::ivec2& ignorePosition)
	{
		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions;
		for (int i = 0; i < adjacentPositions.size(); ++i)
		{
			glm::ivec2 newPosition = position + ALL_DIRECTIONS[i];
			if (newPosition != ignorePosition && isPositionInMapBounds(newPosition))
			{
				adjacentPositions[i] = AdjacentPosition(newPosition);
			}
			else
			{
				adjacentPositions[i] = AdjacentPosition(false);
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

			assert(pathToPosition.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
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

		std::array<AdjacentPosition, ALL_DIRECTIONS.size()> adjacentPositions = getAllAdjacentPositions(position, startingPositionOnGrid);
		for (const auto& adjacentPosition : adjacentPositions)
		{
			if (adjacentPosition.valid && !m_graph.isPositionVisited(adjacentPosition.position))
			{
				m_graph.addToGraph(adjacentPosition.position, position);
				m_frontier.push(adjacentPosition.position);
			}

			if (adjacentPosition.valid && adjacentPosition.position == destinationPositionOnGrid)
			{
				destinationReached = true;
				getPathFromVisitedNodes(startingPositionOnGrid, adjacentPosition.position, pathToPosition, m_graph);
				break;
			}
		}

		assert(m_frontier.size() <= Globals::MAP_SIZE * Globals::MAP_SIZE);
	}

	assert(!pathToPosition.empty());
}