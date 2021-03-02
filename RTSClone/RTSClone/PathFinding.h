#pragma once

#include "glm/glm.hpp"
#include "Globals.h"
#include "Unit.h"
#include "Map.h"
#include "PriorityQueue.h"
#include "Graph.h"
#include "Worker.h"
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
	~PathFinding();

	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	template <class Entity>
	glm::vec3 getClosestAvailablePosition(const Entity& currentEntity, const std::vector<Entity>& entities, const Map& map) const
	{
		constexpr float MAX_RAY_DISTANCE = static_cast<float>(Globals::NODE_SIZE) * 10.0f;
		const std::array<glm::ivec2, 4> DIRECTIONS_ON_GRID =
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
				for (const auto& otherEntity : entities)
				{
					switch (otherEntity.getEntityType())
					{
					case eEntityType::Unit:
						if (!dynamic_cast<const Unit&>(otherEntity).getPathToPosition().empty())
						{
							continue;
						}
						break;
					case eEntityType::Worker:
						if (!dynamic_cast<const Worker&>(otherEntity).getPathToPosition().empty())
						{
							continue;
						}
						break;
					default:
						assert(false);
					}
					if (&currentEntity == &otherEntity)
					{
						continue;
					}
					else if (otherEntity.getAABB().contains(position) || map.isPositionOccupied(position))
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

	bool isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType buildingEntityType, const Map& map,
		glm::vec3& buildPosition, const FactionAI& owningFaction, const BaseHandler& baseHandler);

	bool isUnitPositionAvailable(const glm::vec3& position, const Entity& entity, FactionHandler& factionHandler, 
		const Faction& owningFaction) const;

	bool isPositionInLineOfSight(const glm::vec3& startingPosition, const glm::vec3& targetPosition, const Map& map) const;
	bool isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map) const;
	bool isTargetInLineOfSight(const glm::vec3& startingPosition, const Entity& targetEntity, const Map& map, const AABB& senderAABB) const;

	bool getClosestAvailableEntitySpawnPosition(const EntitySpawnerBuilding& building, const std::vector<Unit>& units, 
		const std::vector<Worker>& workers, const Map& map, glm::vec3& position);

	glm::vec3 getRandomAvailablePositionOutsideAABB(const Entity& senderEntity, const Map& map);

	glm::vec3 getClosestPositionToAABB(const glm::vec3& entityPosition, const AABB& AABB, const Map& map);

	bool setUnitAttackPosition(const Unit& unit, const Entity& targetEntity, std::vector<glm::vec3>& pathToPosition,
		const Map& map, FactionHandler& factionHandler);

	void getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		const Map& map, FactionHandler& factionHandler, const Faction& owningFaction);

	void getPathToPosition(const Worker& entity, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		AdjacentPositions adjacentPositions, const Map& map, const Faction& owningFaction);

	void getPathToPosition(const Worker& entity, const Entity& target, std::vector<glm::vec3>& pathToPosition,
		const Map& map, const Faction& owningFaction);

private:
	PathFinding();
	std::vector<glm::vec3> m_sharedPositionContainer;
	//BFS
	Graph m_graph;
	std::queue<glm::ivec2> m_frontier;
	//A*
	PriorityQueue m_openQueue;
	PriorityQueue m_closedQueue;

	void onNewMapSize(const GameMessages::NewMapSize& gameMessage);
};