#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Globals.h"
#include "GameMessageType.h"
#include <array>

class MapNode
{
public:
	MapNode();
	MapNode(bool collidable, int entityID);

	bool isCollidable() const;
	int getEntityID() const;

private:
	bool m_collidable;
	int m_entityID;
};

namespace GameMessages
{
	struct AddToMap;
	struct RemoveFromMap;
	struct NewMapSize;
}
class AABB;
class Map : private NonCopyable, private NonMovable
{
public:
	Map();
	~Map();

	MapNode getNode(const glm::vec3& position) const;
	MapNode getNode(glm::ivec2 position) const;
	const glm::ivec2& getSize() const;
	bool isWithinBounds(const AABB& AABB) const;
	bool isWithinBounds(const glm::vec3& position) const;
	bool isWithinBounds(const glm::ivec2& position) const;
	bool isAABBOccupied(const AABB& AABB) const;
	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;

private:
	glm::ivec2 m_size;
	std::vector<MapNode> m_map;
	
	void addEntityToMap(const GameMessages::AddToMap& gameMessage);
	void removeEntityFromMap(const GameMessages::RemoveFromMap& gameMessage);
	void setSize(const GameMessages::NewMapSize& gameMessage);
};