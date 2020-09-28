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
class AABB;
class Map : private NonMovable, private NonCopyable
{
public:
	Map();
	~Map();

	bool isWithinBounds(const AABB& AABB) const;
	bool isWithinBounds(const glm::ivec2& position) const;
	bool isWithinBounds(const glm::vec3& position) const;
	const glm::ivec2& getSize() const;
	bool isAABBOccupied(const AABB& AABB) const;
	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;

	void setSize(const glm::ivec2& size);

private:
	glm::ivec2 m_size;
	std::vector<bool> m_map;
	
	void addEntityToMap(const GameMessages::MapModification<eGameMessageType::AddEntityToMap>& gameEvent);
	void removeEntityFromMap(const GameMessages::MapModification<eGameMessageType::RemoveEntityFromMap>& gameEvent);
};