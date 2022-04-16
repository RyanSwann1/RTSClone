#pragma once

#include "glm/glm.hpp"
#include "Core/Globals.h"
#include "Entities/Unit.h"
#include "Core/Map.h"
#include "Core/Graph.h"
#include "Entities/Worker.h"
#include "MinHeap.h"
#include "Events/GameMessenger.h"
#include <vector>
#include <queue>
#include <array>
#include <functional>

struct ThetaStarGraphNode
{
	ThetaStarGraphNode();
	ThetaStarGraphNode(glm::ivec2 position, glm::ivec2 cameFrom, float g, float h);

	float getF() const;                                                                                                                                                                                                                                                                                             

	glm::ivec2 position;
	glm::ivec2 cameFrom;
	float g;
	float h;
};

namespace GameMessages
{
	struct MapSize;
}
class Faction;
class Entity;
class Worker;
class Unit;
class Map;
class FactionAI;
class BaseHandler;
class EntitySpawnerBuilding;
class PathFinding 
{
public:
	PathFinding(const PathFinding&) = delete;
	PathFinding& operator=(const PathFinding&) = delete;
	PathFinding(PathFinding&&) = delete;
	PathFinding& operator=(PathFinding&&) = delete;

	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	bool getClosestAvailablePosition(const Worker& worker, const std::vector<Worker>& workers, 
		const Map& map, glm::vec3& position);

	bool isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType buildingEntityType, const Map& map,
		glm::vec3& buildPosition, const FactionAI& owningFaction, const BaseHandler& baseHandler);

	bool isPositionInLineOfSight(glm::ivec2 startingPositionOnGrid, glm::ivec2 targetPositionOnGrid, const Map& map, const Entity& entity) const;
	bool isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map) const;
	bool isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map, const AABB& senderAABB) const;
	bool isTargetInLineOfSight(const Unit& unit, const Entity& targetEntity, const Map& map) const;

	bool getClosestAvailableEntitySpawnPosition(const EntitySpawnerBuilding& building, const Map& map, glm::vec3& position);

	bool getRandomPositionOutsideAABB(const Entity& building, const Map& map, glm::vec3& positionOutsideAABB);

	glm::vec3 getClosestPositionToAABB(const glm::vec3& entityPosition, const AABB& AABB, const Map& map);

	bool setUnitAttackPosition(const Unit& unit, const Entity& targetEntity, std::vector<glm::vec3>& pathToPosition,
		const Map& map);

	void getPathToPosition(const Entity& entity, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		const Map& map, AdjacentPositions adjacentPositions);

private:
	PathFinding();
	std::vector<glm::vec3> m_sharedContainer;
	Graph m_bfsGraph;
	//ThetaStar
	std::vector<ThetaStarGraphNode> m_thetaGraph;
	MinHeap m_thetaFrontier;
	BroadcasterSub<GameMessages::MapSize> m_onNewMapSizeID;

	void expandFrontier(const MinHeapNode& currentNode, const Map& map, glm::ivec2 destinationOnGrid, AdjacentPositions adjacentPositions,
		const Entity& entity);
	void onNewMapSize(GameMessages::MapSize&& gameMessage);
};