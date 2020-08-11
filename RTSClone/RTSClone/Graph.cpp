#include "Graph.h"

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

void Graph::reset(std::queue<glm::ivec2>& frontier)
{
	for (auto& i : m_graph)
	{
		i = GraphNode();
	}

	std::queue<glm::ivec2> empty;
	frontier.swap(empty);
}

void Graph::addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition)
{
	assert(Globals::isPositionInMapBounds(position) && !m_graph[Globals::convertTo1D(position)].isVisited());
	if (Globals::isPositionInMapBounds(position) && !m_graph[Globals::convertTo1D(position)].isVisited())
	{
		m_graph[Globals::convertTo1D(position)] = GraphNode(cameFromPosition);
	}
}

bool Graph::isEmpty() const
{
	return m_graph.empty();
}

const glm::ivec2& Graph::getPreviousPosition(const glm::ivec2& position) const
{
	assert(Globals::isPositionInMapBounds(position) && m_graph[Globals::convertTo1D(position)].isVisited());
	if (Globals::isPositionInMapBounds(position) && m_graph[Globals::convertTo1D(position)].isVisited())
	{
		return m_graph[Globals::convertTo1D(position)].getCameFrom();
	}
}

bool Graph::isPositionVisited(const glm::ivec2& position) const
{
	assert(Globals::isPositionInMapBounds(position));
	if (Globals::isPositionInMapBounds(position))
	{
		return m_graph[Globals::convertTo1D(position)].isVisited();
	}
}

void Graph::resetGraph()
{

}