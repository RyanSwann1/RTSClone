#pragma once

#include "Globals.h"
#include "GameMessenger.h"
#include "SceneryGameObject.h"
#include "Base.h"

namespace GameMessages
{
	struct AddAABBToMap;
	struct RemoveAABBFromMap;
	struct AddUnitPositionToMap;
	struct RemoveUnitPositionFromMap;
}
class AABB;
class Map 
{
public:
	Map(const std::vector<SceneryGameObject>& sceneryGameObjects, const std::vector<Base>& bases, glm::ivec2 size);

	bool isCollidable(const glm::vec3& position) const;
	const glm::ivec2& getSize() const;
	bool isWithinBounds(const AABB& AABB) const;
	bool isWithinBounds(const glm::vec3& position) const;
	bool isWithinBounds(const glm::ivec2& position) const;
	bool isAABBOccupied(const AABB& AABB) const;
	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;
	bool isPositionOnUnitMapAvailable(glm::ivec2 position, int senderID) const;

	void editMap(const AABB& AABB, bool occupyAABB);

private:
	glm::ivec2 m_size;
	std::vector<bool> m_map;
	std::vector<int> m_unitMap;

	BroadcasterSub<GameMessages::AddAABBToMap> m_addABBID;
	BroadcasterSub<GameMessages::RemoveAABBFromMap> m_removeABBBFromMapID;
	BroadcasterSub<GameMessages::AddUnitPositionToMap> m_addUnitPositionToMapID;
	BroadcasterSub<GameMessages::RemoveUnitPositionFromMap> m_removeUnitPositionFromMapID;

	void addAABB(GameMessages::AddAABBToMap&& message);
	void removeAABB(GameMessages::RemoveAABBFromMap&& message);
	void addUnitPosition(GameMessages::AddUnitPositionToMap&& message);
	void removeUnitPosition(GameMessages::RemoveUnitPositionFromMap&& message);

	int getIDOnUnitMap(glm::ivec2 position) const;

	void editUnitMap(const glm::vec3& position, int ID, bool occupy);
};