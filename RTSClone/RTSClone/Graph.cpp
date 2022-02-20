#include "Graph.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "Map.h"
#include <algorithm>

//Graph
Graph::Graph()
	: m_onNewMapSizeID([this](const GameMessages::NewMapSize& gameMessage) { return new_map_size(gameMessage); })
{}

void Graph::add(glm::ivec2 position, glm::ivec2 cameFromPosition, const Map& map)
{
	assert(map.isWithinBounds(position) && !m_graph[Globals::convertTo1D(position, map.getSize())]);
	if (map.isWithinBounds(position) && !m_graph[Globals::convertTo1D(position, map.getSize())])
	{
		m_graph[Globals::convertTo1D(position, map.getSize())] = cameFromPosition;
		m_frontier.push(position);
		assert(m_frontier.size() <= (m_size.x * m_size.y));
	}
}

void Graph::new_map_size(const GameMessages::NewMapSize& gameMessage)
{
	m_size = gameMessage.mapSize;
	m_graph.resize(static_cast<size_t>(m_size.x * m_size.y));
	m_graph.assign(m_graph.size(), {});
}

void Graph::reset(glm::ivec2 startingPosition)
{
	m_frontier = {};
	m_frontier.push(startingPosition);
	m_graph.assign(m_graph.size(), {});
}

bool Graph::is_frontier_empty() const
{
	return m_frontier.empty();
}

glm::ivec2 Graph::pop_frontier()
{
	assert(!m_frontier.empty());
	glm::ivec2 position = m_frontier.front();
	m_frontier.pop();
	return position;
}

bool Graph::is_position_visited(glm::ivec2 position, const Map& map) const
{
	assert(map.isWithinBounds(position));
	if (map.isWithinBounds(position))
	{
		return m_graph[Globals::convertTo1D(position, map.getSize())].has_value();
	}

	return false;
}