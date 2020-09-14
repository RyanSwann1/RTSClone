#include "Map.h"
#include "AABB.h"
#include "GameMessenger.h"
#include "GameMessages.h"

Map::Map()
	: m_map()
{
	for (auto& i : m_map)
	{
		i = false;
	}

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

bool Map::isPositionOccupied(const glm::vec3& position) const
{
	if (Globals::isPositionInMapBounds(position))
	{
		glm::ivec2 positionOnGrid = Globals::convertToGridPosition(position);
		return m_map[Globals::convertTo1D(positionOnGrid)];
	}

	return true;
}

bool Map::isPositionOccupied(const glm::ivec2& position) const
{
	if (Globals::isPositionInMapBounds(position))
	{
		return m_map[Globals::convertTo1D(position)];
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
			assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid)] = true;
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
			assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid)] = false;
		}
	}
}