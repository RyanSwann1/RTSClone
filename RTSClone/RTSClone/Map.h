#pragma once

#include "Globals.h"
#include "GameMessenger.h"

namespace GameMessages
{
	struct AddAABBToMap;
	struct RemoveAABBFromMap;
	struct AddUnitPositionToMap;
	struct RemoveUnitPositionFromMap;
	struct NewMapSize;
}
class AABB;
class Map 
{
public:
	Map();
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;
	Map(Map&&) = default;
	Map& operator=(Map&&) = default;

	bool isCollidable(const glm::vec3& position) const;
	const glm::ivec2& getSize() const;
	bool isWithinBounds(const AABB& AABB) const;
	bool isWithinBounds(const glm::vec3& position) const;
	bool isWithinBounds(const glm::ivec2& position) const;
	bool isAABBOccupied(const AABB& AABB) const;
	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;
	
	bool isPositionOnUnitMapAvailable(glm::ivec2 position, int senderID) const;

private:
	glm::ivec2 m_size;
	std::vector<bool> m_map;
	std::vector<int> m_unitMap;

	GameMessengerSubscriber<GameMessages::AddAABBToMap> m_addABBID;
	GameMessengerSubscriber<GameMessages::RemoveAABBFromMap> m_removeABBBFromMapID;
	GameMessengerSubscriber<GameMessages::AddUnitPositionToMap> m_addUnitPositionToMapID;
	GameMessengerSubscriber<GameMessages::RemoveUnitPositionFromMap> m_removeUnitPositionFromMapID;
	GameMessengerSubscriber<GameMessages::NewMapSize> m_setSizeID;

	void addAABB(const GameMessages::AddAABBToMap& message);
	void removeAABB(const GameMessages::RemoveAABBFromMap& message);
	void addUnitPosition(const GameMessages::AddUnitPositionToMap& message);
	void removeUnitPosition(const GameMessages::RemoveUnitPositionFromMap& message);
	void setSize(const GameMessages::NewMapSize& message);

	int getIDOnUnitMap(glm::ivec2 position) const;
	void editMap(const AABB& AABB, bool occupyAABB);
	void editUnitMap(const glm::vec3& position, int ID, bool occupy);
};