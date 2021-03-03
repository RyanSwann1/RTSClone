#pragma once

#include "Globals.h"

namespace GameMessages
{
	struct AddBuildingToMap;
	struct RemoveBuildingFromMap;
	struct AddMineralToMap;
	struct RemoveMineralFromMap;
	struct AddSceneryGameObjectToMap;
	struct RemoveSceneryGameObjectFromMap;
	struct NewMapSize;
}
class AABB;
class Map 
{
public:
	Map();
	Map(const Map&) = delete;
	Map& operator=(const Map&) = delete;
	Map(Map&&) = delete;
	Map& operator=(Map&&) = delete;
	~Map();

	bool isCollidable(const glm::vec3& position) const;
	const glm::ivec2& getSize() const;
	bool isWithinBounds(const AABB& AABB) const;
	bool isWithinBounds(const glm::vec3& position) const;
	bool isWithinBounds(const glm::ivec2& position) const;
	bool isAABBOccupied(const AABB& AABB) const;
	bool isPositionOccupied(const glm::vec3& position) const;
	bool isPositionOccupied(const glm::ivec2& position) const;

private:
	glm::ivec2 m_size;
	std::vector<bool> m_map;
	
	void addEntityToMap(const GameMessages::AddBuildingToMap& message);
	void removeEntityFromMap(const GameMessages::RemoveBuildingFromMap& message);
	void addMineralToMap(const GameMessages::AddMineralToMap& message);
	void removeMineralFromMap(const GameMessages::RemoveMineralFromMap& message);
	void addSceneryToMap(const GameMessages::AddSceneryGameObjectToMap& message);
	void removeSceneryFromMap(const GameMessages::RemoveSceneryGameObjectFromMap& message);

	void setSize(const GameMessages::NewMapSize& message);
	void editMap(const AABB& AABB, bool occupyAABB);
};