#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include "GameEventType.h"
#include <array>

namespace GameEvents
{
	template <eGameEventType type>
	struct MapModification;
}
struct AABB;
class Map : private NonMovable, private NonCopyable
{
public:
	Map();
	~Map();

	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;

private:
	std::array<bool, static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)> m_map;
	
	void addEntityToMap(const GameEvents::MapModification<eGameEventType::AddEntityToMap>& gameEvent);
	void removeEntityFromMap(const GameEvents::MapModification<eGameEventType::RemoveEntityFromMap>& gameEvent);
};