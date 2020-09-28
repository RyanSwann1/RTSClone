#include "Map.h"
#include "AABB.h"
#include "GameMessenger.h"
#include "GameMessages.h"

Map::Map()
	: m_size(),
	m_map()
{
	GameMessenger::getInstance().subscribe<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>(
		[&](const GameMessages::MapModification<eGameMessageType::AddEntityToMap>& gameEvent) { return addEntityToMap(gameEvent); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>(
		[&](const GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent) { return removeEntityFromMap(gameEvent); }, this);
}

Map::~Map()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::MapModification<eGameMessageType::AddEntityToMap>>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>>(this);
}

void Map::setSize(const glm::ivec2& size)
{
	m_size = size;
	m_map.resize(static_cast<size_t>(size.x * size.y, false));
}

bool Map::isWithinBounds(const AABB& AABB) const
{
	return AABB.getLeft() >= 0 &&
	AABB.getRight() < m_size.x * Globals::NODE_SIZE &&
	AABB.getBack() >= 0 &&
	AABB.getForward() < m_size.y * Globals::NODE_SIZE;
}

bool Map::isWithinBounds(const glm::ivec2& position) const
{
	return position.x >= 0 &&
	position.x < m_size.x &&
	position.y >= 0 &&
	position.y < m_size.y;
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

const glm::ivec2& Map::getSize() const
{
	return m_size;
}

//TODO: Check - potential error
bool Map::isAABBOccupied(const AABB& AABB) const
{
	for (int x = static_cast<int>(AABB.getLeft()); x <= static_cast<int>(AABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(AABB.getBack()); y <= static_cast<int>(AABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			if (m_map[Globals::convertTo1D(positionOnGrid, m_size)])
			{
				return false;
			}
		}
	}

	return true;
}

bool Map::isPositionOccupied(const glm::vec3& position) const
{
	if (isWithinBounds(position))
	{
		glm::ivec2 positionOnGrid = Globals::convertToGridPosition(position);
		return m_map[Globals::convertTo1D(positionOnGrid, m_size)];
		//return m_map[Globals::convertTo1D(positionOnGrid)];
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

void Map::addEntityToMap(const GameMessages::MapModification<eGameMessageType::AddEntityToMap>& gameEvent)
{
	for (int x = static_cast<int>(gameEvent.entityAABB.getLeft()); x < static_cast<int>(gameEvent.entityAABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(gameEvent.entityAABB.getBack()); y < static_cast<int>(gameEvent.entityAABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			assert(isWithinBounds(positionOnGrid));// Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid, m_size)] = true;
			//m_map[Globals::convertTo1D(positionOnGrid)] = true;
		}
	}
}

void Map::removeEntityFromMap(const GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent)
{
	for (int x = static_cast<int>(gameEvent.entityAABB.getLeft()); x < static_cast<int>(gameEvent.entityAABB.getRight()); ++x)
	{
		for (int y = static_cast<int>(gameEvent.entityAABB.getBack()); y < static_cast<int>(gameEvent.entityAABB.getForward()); ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, Globals::GROUND_HEIGHT, y });
			assert(isWithinBounds(positionOnGrid));
			//assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid, m_size)] = false;
		}
	}
}