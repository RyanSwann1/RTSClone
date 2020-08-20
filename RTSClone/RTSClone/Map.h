#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include "GameMessageType.h"
#include <array>

namespace GameMessages
{
	template <eGameMessageType type>
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
	
	void addEntityToMap(const GameMessages::MapModification<eGameMessageType::AddEntityToMap>& gameEvent);
	void removeEntityFromMap(const GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent);
};