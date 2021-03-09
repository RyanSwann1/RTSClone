#include "Map.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Entity.h"
#include "Mineral.h"
#include "SceneryGameObject.h"
#include <assert.h>

Map::Map()
	: m_size(),
	m_map()
{
	subscribeToMessenger<GameMessages::AddAABBToMap>([this](const GameMessages::AddAABBToMap& message) { return addAABB(message); }, this);
	subscribeToMessenger<GameMessages::RemoveAABBFromMap>([this](const GameMessages::RemoveAABBFromMap& message) { return removeAABB(message); }, this);
	subscribeToMessenger<GameMessages::AddUnitPositionToMap>([this](const GameMessages::AddUnitPositionToMap& message) { return addUnitPosition(message); }, this);
	subscribeToMessenger<GameMessages::RemoveUnitPositionFromMap>(
		[this](const GameMessages::RemoveUnitPositionFromMap& message) { return removeUnitPosition(message); }, this);

	subscribeToMessenger<GameMessages::NewMapSize>(
		[this](const GameMessages::NewMapSize& gameMessage) { return setSize(gameMessage); }, this);
}

Map::~Map()
{
	unsubscribeToMessenger<GameMessages::AddAABBToMap>(this);
	unsubscribeToMessenger<GameMessages::RemoveAABBFromMap>(this);
	unsubscribeToMessenger<GameMessages::AddUnitPositionToMap>(this);
	unsubscribeToMessenger<GameMessages::RemoveUnitPositionFromMap>(this);
	unsubscribeToMessenger<GameMessages::NewMapSize>(this);
}

bool Map::isCollidable(const glm::vec3& position) const
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
	for (int x = static_cast<int>(AABB.getLeft()); x < static_cast<int>(AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(AABB.getBack()); y < static_cast<int>(AABB.getForward()); ++y)
		{
			if (m_map[Globals::convertTo1D(Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y }), m_size)])
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
		return m_map[Globals::convertTo1D(Globals::convertToGridPosition(position), m_size)];
	}

	return true;
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	if (isWithinBounds(position))
	{
		return m_map[Globals::convertTo1D(position, m_size)];
	}

	return true;
}

bool Map::isPositionOnUnitMapAvailable(glm::ivec2 position, int senderID) const
{
	if (isWithinBounds(position))
	{
		int ID = m_unitMap[Globals::convertTo1D(position, m_size)];
		return ID == senderID || ID == Globals::INVALID_ENTITY_ID;
	}

	return false;
}

void Map::addAABB(const GameMessages::AddAABBToMap& message)
{
	editMap(message.aabb, true);
}

void Map::removeAABB(const GameMessages::RemoveAABBFromMap& message)
{
	editMap(message.aabb, false);
}

void Map::addUnitPosition(const GameMessages::AddUnitPositionToMap& message)
{
	editUnitMap(message.position, message.ID, true);
}

void Map::removeUnitPosition(const GameMessages::RemoveUnitPositionFromMap& message)
{
	editUnitMap(message.position, message.ID, false);
}

void Map::setSize(const GameMessages::NewMapSize& gameMessage)
{
	m_size = gameMessage.mapSize;
	m_map.resize(static_cast<size_t>(m_size.x) * static_cast<size_t>(m_size.y), false);
	m_unitMap.resize(static_cast<size_t>(m_size.x) * static_cast<size_t>(m_size.y), Globals::INVALID_ENTITY_ID);
}

int Map::getIDOnUnitMap(glm::ivec2 position) const
{
	if (isWithinBounds(position))
	{
		return m_unitMap[Globals::convertTo1D(position, m_size)];
	}

	return Globals::INVALID_ENTITY_ID;
}

void Map::editMap(const AABB& AABB, bool occupyAABB)
{
	for (int x = static_cast<int>(AABB.getLeft()); x < static_cast<int>(AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(AABB.getBack()); y < static_cast<int>(AABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			assert(isWithinBounds(positionOnGrid));
			if (isWithinBounds(positionOnGrid))
			{
				m_map[Globals::convertTo1D(positionOnGrid, m_size)] = occupyAABB;
			}
		}
	}
}

void Map::editUnitMap(const glm::vec3& position, int ID, bool occupy)
{
	assert(isWithinBounds(position));
	glm::ivec2 positionOnGrid = Globals::convertToGridPosition(position);
	int existingUnitID = getIDOnUnitMap(positionOnGrid);
	assert(existingUnitID == Globals::INVALID_ENTITY_ID || existingUnitID == ID);

	if (occupy)
	{
		m_unitMap[Globals::convertTo1D(positionOnGrid, m_size)] = ID;
	}
	else
	{
		m_unitMap[Globals::convertTo1D(positionOnGrid, m_size)] = Globals::INVALID_ENTITY_ID;
	}
}