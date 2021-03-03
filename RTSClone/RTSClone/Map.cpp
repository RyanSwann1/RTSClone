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
	subscribeToMessenger<GameMessages::AddBuildingToMap>([this](const GameMessages::AddBuildingToMap& message) { return addEntityToMap(message); }, this);
	subscribeToMessenger<GameMessages::RemoveBuildingFromMap>(
		[this](const GameMessages::RemoveBuildingFromMap& message) { return removeEntityFromMap(message); }, this);

	subscribeToMessenger<GameMessages::AddMineralToMap>([this](const GameMessages::AddMineralToMap& message) { return addMineralToMap(message); }, this);
	subscribeToMessenger<GameMessages::RemoveMineralFromMap>(
		[this](const GameMessages::RemoveMineralFromMap& message) { return removeMineralFromMap(message); }, this);

	subscribeToMessenger<GameMessages::AddSceneryGameObjectToMap>([this](const GameMessages::AddSceneryGameObjectToMap& message)
		{ return addSceneryToMap(message); }, this);
	subscribeToMessenger<GameMessages::RemoveSceneryGameObjectFromMap>(
		[this](const GameMessages::RemoveSceneryGameObjectFromMap & message) { return removeSceneryFromMap(message); }, this);

	subscribeToMessenger<GameMessages::NewMapSize>(
		[this](const GameMessages::NewMapSize& gameMessage) { return setSize(gameMessage); }, this);
}

Map::~Map()
{
	unsubscribeToMessenger<GameMessages::AddBuildingToMap>(this);
	unsubscribeToMessenger<GameMessages::RemoveBuildingFromMap>(this);
	unsubscribeToMessenger<GameMessages::AddMineralToMap>(this);
	unsubscribeToMessenger<GameMessages::RemoveMineralFromMap>(this);
	unsubscribeToMessenger<GameMessages::AddSceneryGameObjectToMap>(this);
	unsubscribeToMessenger<GameMessages::RemoveSceneryGameObjectFromMap>(this);
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

void Map::addEntityToMap(const GameMessages::AddBuildingToMap& message)
{
	editMap(message.entity.getAABB(), true);
}

void Map::removeEntityFromMap(const GameMessages::RemoveBuildingFromMap& message)
{
	editMap(message.entity.getAABB(), false);
}

void Map::addMineralToMap(const GameMessages::AddMineralToMap& message)
{
	editMap(message.mineral.getAABB(), true);
}

void Map::removeMineralFromMap(const GameMessages::RemoveMineralFromMap& message)
{
	editMap(message.mineral.getAABB(), false);
}

void Map::addSceneryToMap(const GameMessages::AddSceneryGameObjectToMap & message)
{
	editMap(message.gameObject.getAABB(), true);
}

void Map::removeSceneryFromMap(const GameMessages::RemoveSceneryGameObjectFromMap & message)
{
	editMap(message.gameObject.getAABB(), false);
}

void Map::setSize(const GameMessages::NewMapSize& gameMessage)
{
	m_size = gameMessage.mapSize;
	m_map.resize(static_cast<size_t>(m_size.x) * static_cast<size_t>(m_size.y), false);
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