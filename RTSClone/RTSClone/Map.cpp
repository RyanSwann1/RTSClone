#include "Map.h"
#include "AABB.h"
#include "GameMessenger.h"
#include "GameEvents.h"

Map::Map()
	: m_map()
{
	for (auto& i : m_map)
	{
		i = false;
	}

	GameMessenger::getInstance().subscribe<GameEvents::MapModification<eGameMessageType::AddEntityToMap>>(
		[&](const GameEvents::MapModification<eGameMessageType::AddEntityToMap>& gameEvent) { return addEntityToMap(gameEvent); }, this);

	GameMessenger::getInstance().subscribe<GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>>(
		[&](const GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent) { return removeEntityFromMap(gameEvent); }, this);
}

Map::~Map()
{
	GameMessenger::getInstance().unsubscribe<GameEvents::MapModification<eGameMessageType::AddEntityToMap>>(this);
	GameMessenger::getInstance().unsubscribe<GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>>(this);
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

void Map::addEntityToMap(const GameEvents::MapModification<eGameMessageType::AddEntityToMap>& gameEvent)
{
	for (int x = gameEvent.entityAABB.m_left; x < gameEvent.entityAABB.m_right; ++x)
	{
		for (int y = gameEvent.entityAABB.m_back; y < gameEvent.entityAABB.m_forward; ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, 0.0f, y });
			assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid)] = true;
		}
	}
}

void Map::removeEntityFromMap(const GameEvents::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent)
{
	for (int x = gameEvent.entityAABB.m_left; x < gameEvent.entityAABB.m_right; ++x)
	{
		for (int y = gameEvent.entityAABB.m_back; y < gameEvent.entityAABB.m_forward; ++y)
		{
			glm::ivec2 positionOnGrid = Globals::convertToGridPosition({ x, 0.0f, y });
			assert(Globals::isPositionInMapBounds(positionOnGrid));
			m_map[Globals::convertTo1D(positionOnGrid)] = false;
		}
	}
}
