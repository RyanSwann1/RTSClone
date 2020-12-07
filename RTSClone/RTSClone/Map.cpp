#include "Map.h"
#include "AABB.h"
#include "GameMessenger.h"
#include "GameMessages.h"

//MapNode
MapNode::MapNode()
	: m_collidable(false),
	m_entityID(Globals::INVALID_ENTITY_ID)
{}

MapNode::MapNode(bool collidable, int entityID)
	: m_collidable(collidable),
	m_entityID(entityID)
{}

bool MapNode::isCollidable() const
{
	return m_collidable;
}

int MapNode::getEntityID() const
{
	return m_entityID;
}

//Map
Map::Map()
	: m_size(),
	m_map()
{
	GameMessenger::getInstance().subscribe<GameMessages::AddToMap>(
		[this](const GameMessages::AddToMap& gameMessage) { return addEntityToMap(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::RemoveFromMap>(
		[this](const GameMessages::RemoveFromMap& gameMessage) { return removeEntityFromMap(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::NewMapSize>(
		[this](const GameMessages::NewMapSize& gameMessage) { return setSize(gameMessage); }, this);
}

Map::~Map()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::AddToMap>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::RemoveFromMap>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::NewMapSize>(this);
}

const MapNode& Map::getNode(const glm::vec3& position) const
{
	assert(isWithinBounds(position));
	return m_map[Globals::convertTo1D(Globals::convertToGridPosition(position), m_size)];
}

const glm::ivec2& Map::getSize() const
{
	return m_size;
}

bool Map::isWithinBounds(const AABB& AABB) const
{
	return AABB.getLeft() >= 0 &&
		AABB.getRight() < m_size.x * Globals::NODE_SIZE &&
		AABB.getBack() >= 0 &&
		AABB.getForward() < m_size.y * Globals::NODE_SIZE;
}

bool Map::isWithinBounds(const glm::vec3& position) const
{
	return position.x >= 0 &&
	position.x < m_size.x * Globals::NODE_SIZE &&
	position.y >= 0 &&
	position.y < m_size.y * Globals::NODE_SIZE &&
	position.z >= 0 &&
	position.z < m_size.y * Globals::NODE_SIZE;
}

bool Map::isWithinBounds(const glm::ivec2& position) const
{
	return position.x >= 0 &&
	position.x < m_size.x &&
	position.y >= 0 &&
	position.y < m_size.y;
}

bool Map::isAABBOccupied(const AABB& AABB) const
{
	for (int x = static_cast<int>(AABB.getLeft()); x <= static_cast<int>(AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(AABB.getBack()); y <= static_cast<int>(AABB.getForward()); ++y)
		{
			if (m_map[Globals::convertTo1D(Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y }), m_size)].isCollidable())
			{
				return true;
			}
		}
	}

	return false;
}

bool Map::isPositionOccupied(const glm::vec3& position) const
{
	if (isWithinBounds(position))
	{
		return m_map[Globals::convertTo1D(Globals::convertToGridPosition(position), m_size)].isCollidable();
	}

	return true;
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	if (isWithinBounds(position))
	{
		return m_map[Globals::convertTo1D(position, m_size)].isCollidable();
	}

	return true;
}

void Map::addEntityToMap(const GameMessages::AddToMap& gameMessage)
{
	for (int x = static_cast<int>(gameMessage.AABB.getLeft()); x < static_cast<int>(gameMessage.AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(gameMessage.AABB.getBack()); y < static_cast<int>(gameMessage.AABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			assert(isWithinBounds(positionOnGrid));
			if (isWithinBounds(positionOnGrid))
			{
				m_map[Globals::convertTo1D(positionOnGrid, m_size)] = { true, gameMessage.entityID };
			}
		}
	}
}

void Map::removeEntityFromMap(const GameMessages::RemoveFromMap& gameMessage)
{
	for (int x = static_cast<int>(gameMessage.AABB.getLeft()); x < static_cast<int>(gameMessage.AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(gameMessage.AABB.getBack()); y < static_cast<int>(gameMessage.AABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			assert(isWithinBounds(positionOnGrid));
			if (isWithinBounds(positionOnGrid))
			{
				m_map[Globals::convertTo1D(positionOnGrid, m_size)] = { false, Globals::INVALID_ENTITY_ID };
			}
		}
	}
}

void Map::setSize(const GameMessages::NewMapSize& gameMessage)
{
	m_size = gameMessage.mapSize;
	m_map.resize(static_cast<size_t>(m_size.x * m_size.y), { false, Globals::INVALID_ENTITY_ID });
}