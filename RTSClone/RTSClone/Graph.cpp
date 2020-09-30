#include "Graph.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "Map.h"

//GraphNode
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
	: m_size(),
	m_graph()
{
	GameMessenger::getInstance().subscribe<GameMessages::NewMapSize>(
		[this](const GameMessages::NewMapSize& gameMessage) { return onNewMapSize(gameMessage); }, this);
}

Graph::~Graph()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::NewMapSize>(this);
}

void Graph::reset(std::queue<glm::ivec2>& frontier)
{
	reset();
	std::queue<glm::ivec2> empty;
	frontier.swap(empty);
}

void Graph::addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition, const Map& map)
{
	assert(map.isWithinBounds(position) && !m_graph[Globals::convertTo1D(position, map.getSize())].isVisited());
	if (map.isWithinBounds(position) && !m_graph[Globals::convertTo1D(position, map.getSize())].isVisited())
	{
		m_graph[Globals::convertTo1D(position, map.getSize())] = GraphNode(cameFromPosition);
	}
}

void Graph::onNewMapSize(const GameMessages::NewMapSize& gameMessage)
{
	m_size = gameMessage.mapSize;
	reset();
}

void Graph::reset()
{
	m_graph.clear();
	m_graph.resize(static_cast<size_t>(m_size.x * m_size.y), {});
}

bool Graph::isEmpty() const
{
	return m_graph.empty();
}

const glm::ivec2& Graph::getPreviousPosition(const glm::ivec2& position, const Map& map) const
{
	assert(map.isWithinBounds(position) && m_graph[Globals::convertTo1D(position, map.getSize())].isVisited());
	if (map.isWithinBounds(position) && m_graph[Globals::convertTo1D(position, map.getSize())].isVisited())
	{
		return m_graph[Globals::convertTo1D(position, map.getSize())].getCameFrom();
	}
}

bool Graph::isPositionVisited(const glm::ivec2& position, const Map& map) const
{
	assert(map.isWithinBounds(position));
	if (map.isWithinBounds(position))
	{
		return m_graph[Globals::convertTo1D(position, map.getSize())].isVisited();
	}
}