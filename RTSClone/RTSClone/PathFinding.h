#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "Globals.h"
#include "Unit.h"
#include "Map.h"
#include "PriorityQueue.h"
#include "Graph.h"
#include <vector>
#include <queue>
#include <array>
#include <functional>
#include <list>

namespace GameMessages
{
	struct NewMapSize;
}
struct Model;
class Faction;
class Entity;
class Worker;
class Unit;
class Map;
class FactionHandler;
class PathFinding : private NonCopyable, private NonMovable
{
public:
	PathFinding();
	~PathFinding();

	template <class Entity>
	glm::vec3 getClosestAvailablePosition(const Entity& currentEntity, const std::list<Entity>& entities, const Map& map) const
	{
		assert(currentEntity.getCurrentState() == eUnitState::Idle);
		constexpr float MAX_RAY_DISTANCE = static_cast<float>(Globals::NODE_SIZE) * 10.0f;
		constexpr std::array<glm::ivec2, 4> DIRECTIONS_ON_GRID =
		{
			glm::ivec2(0, 1),
			glm::ivec2(1, 0),
			glm::ivec2(0, -1),
			glm::ivec2(-1, 0),
		};

		float distance = std::numeric_limits<float>::max();
		glm::vec3 shortestDistancePosition = currentEntity.getPosition();

		for (const auto& direction : DIRECTIONS_ON_GRID)
		{
			glm::vec3 position = currentEntity.getPosition();
			for (float ray = static_cast<float>(Globals::NODE_SIZE); ray <= MAX_RAY_DISTANCE; ray += static_cast<float>(Globals::NODE_SIZE))
			{
				position = position + glm::normalize(glm::vec3(direction.x, Globals::GROUND_HEIGHT, direction.y)) * ray;

				bool collision = false;
				for (const auto& otherEntities : entities)
				{
					if (&currentEntity == &otherEntities || otherEntities.getCurrentState() != eUnitState::Idle)
					{
						continue;
					}
					else if (otherEntities.getAABB().contains(position) || map.isPositionOccupied(position))
					{
						collision = true;
						break;
					}
				}

				if (!collision && glm::distance(currentEntity.getPosition(), position) < distance)
				{
					distance = glm::distance(currentEntity.getPosition(), position);
					shortestDistancePosition = position;
				}
			}
		}

		assert(distance < std::numeric_limits<float>::max() && 
			shortestDistancePosition != currentEntity.getPosition());

		return shortestDistancePosition;
	}

	bool isBuildingSpawnAvailable(const glm::vec3& startingPosition, const Model& model, const Map& map,
		glm::vec3& buildPosition, float minDistanceFromHQ, float maxDistanceFromHQ, float distanceFromMinerals, const Faction& owningFaction);

	bool isPositionAvailable(const glm::vec3& nodePosition, const Map& map, const std::list<Unit>& units, const std::list<Worker>& workers, 
		int senderID = Globals::INVALID_ENTITY_ID) const;

	bool isTargetInLineOfSight(const glm::vec3& entityPosition, const Entity& targetEntity, const Map& map) const;
	bool isTargetInLineOfSight(const glm::vec3& entityPosition, const Entity& targetEntity, const Map& map, const AABB& senderAABB) const;

	const std::vector<glm::vec3>& getFormationPositions(const glm::vec3& startingPosition, const std::vector<Unit*>& selectedUnits,
		const Map& map);

	glm::vec3 getClosestAvailablePosition(const glm::vec3& startingPosition, const std::list<Unit>& units, 
		const std::list<Worker>& workers, const Map& map);

	glm::vec3 getAvailablePositionOutsideAABB(const Entity& senderEntity, const Map& map);

	glm::vec3 getClosestPositionOutsideAABB(const glm::vec3& entityPosition, const AABB& AABB, const glm::vec3& centrePositionAABB,
		const Map& map);

	glm::vec3 getClosestPositionFromUnitToTarget(const Unit& unit, const Entity& entityTarget, std::vector<glm::vec3>& pathToPosition,
		const Map& map, const AdjacentPositions& adjacentPositions) const;

	void setUnitAttackPosition(const Unit& unit, const Entity& targetEntity, std::vector<glm::vec3>& pathToPosition,
		const Map& map, const std::list<Unit>& units, const FactionHandler& factionHandler);

	void getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		const AdjacentPositions& adjacentPositions, const std::list<Unit>& units, const Map& map);

	void clearAttackPositions();

private:
	std::vector<glm::vec3> m_sharedPositionContainer;
	//BFS
	Graph m_graph;
	std::queue<glm::ivec2> m_frontier;
	//A*
	PriorityQueue m_openQueue;
	PriorityQueue m_closedQueue;

	void onNewMapSize(const GameMessages::NewMapSize& gameMessage);
};