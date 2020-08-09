#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "Globals.h"
#include "Unit.h"
#include "Map.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include <vector>
#include <queue>
#include <array>
#include <unordered_set>

//Pathfinding Optimisations
//https://www.reddit.com/r/gamedev/comments/dk19g6/new_pathfinding_algorithm_factorio/

struct PriorityQueueNode
{
	PriorityQueueNode(const glm::ivec2& position, const glm::ivec2& parentPosition, float g, float h);

	float getF() const;

	bool traversable;
	glm::ivec2 position;
	glm::ivec2 parentPosition;
	float g; //Distance between successor and previous
	float h; //previous.g + Distance to destination
};

const auto nodeCompare = [](const auto& a, const auto& b) -> bool { return b.getF() < a.getF(); };
class PriorityQueue : private std::priority_queue<PriorityQueueNode, std::vector<PriorityQueueNode>, decltype(nodeCompare)>
{
public:
	PriorityQueue(size_t size);

	size_t getSize() const;
	bool isEmpty() const;
	bool contains(const glm::ivec2& position) const;
	const PriorityQueueNode& getTop() const;
	PriorityQueueNode& getNode(const glm::ivec2& position);
	bool isSuccessorNodeValid(const PriorityQueueNode& successorNode) const;

	void add(const PriorityQueueNode& node);
	void popTop();
	void clear();

private:
	std::unordered_set<glm::ivec2> m_addedNodePositions;
};

class GraphNode
{
public:
	GraphNode();
	GraphNode(const glm::ivec2& cameFrom);

	const glm::ivec2& getCameFrom() const;
	bool isVisited() const;

private:
	glm::ivec2 cameFrom;
	bool visited;
};

class Graph : private NonMovable, private NonCopyable
{
public:
	Graph();

	bool isEmpty() const;
	const glm::ivec2& getPreviousPosition(const glm::ivec2& position) const;
	bool isPositionVisited(const glm::ivec2& position) const;

	void resetGraph();
	void addToGraph(const glm::ivec2& position, const glm::ivec2& cameFromPosition);

private:
	std::array<GraphNode, static_cast<size_t>(Globals::MAP_SIZE * Globals::MAP_SIZE)> m_graph;
};

class Harvester;
class Unit;
class Map;
class PathFinding : private NonMovable, private NonCopyable
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	template <class Entity>
	glm::vec3 getClosestPositionOutsideAABB(const Entity& currentEntity, const std::vector<Entity>& entities, const Map& map)
	{
		constexpr float MAX_RAY_DISTANCE = static_cast<float>(Globals::NODE_SIZE) * 10.0f;
		constexpr std::array<glm::ivec2, 4> DIRECTIONS =
		{
			glm::ivec2(0, 1),
			glm::ivec2(1, 0),
			glm::ivec2(0, -1),
			glm::ivec2(-1, 0),
		};

		assert(currentEntity.getCurrentState() == eUnitState::Idle);
		float distance = std::numeric_limits<float>::max();
		glm::vec3 shortestDistancePosition = currentEntity.getPosition();

		for (const auto& direction : DIRECTIONS)
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

		assert(distance < std::numeric_limits<float>::max() && shortestDistancePosition != currentEntity.getPosition());
		return shortestDistancePosition;
	}

	
	std::vector<glm::vec3> getFormationPositions(const glm::vec3& startingPosition, const std::vector<const Unit*> selectedUnits,
		const Map& map);
	glm::vec3 getClosestAvailablePosition(const glm::vec3& startingPosition, const std::vector<Unit>& units, const Map& map);
	void getPathToPosition(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		const Map& map, const std::vector<Unit>& units);
	void getPathToPosition(const glm::vec3& startingPosition, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition,
		const Map& map);
	void getPathToPositionAmongstGroup(const Unit& unit, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition,
		const Map& map, const std::vector<Unit>& units, const std::vector<const Unit*>& selectedUnits);
	void getPathToPositionAmongstGroup(const Unit& unit, const glm::vec3& destinationPosition, std::vector<glm::vec3>& pathToPosition,
	const Map& map);
	void getPathToClosestPositionOutsideAABB(const glm::vec3& entityPosition, const AABB& AABB, const glm::vec3& centrePositionAABB, 
		const Map& map, std::vector<glm::vec3>& pathToPosition);

	void getPathToPositionAStar(const Unit& unit, const glm::vec3& destination, std::vector<glm::vec3>& pathToPosition, const Map& map,
		const std::vector<Unit>& units);

private:
	PathFinding();

	Graph m_graph;
	std::queue<glm::ivec2> m_frontier;
	PriorityQueue m_openQueue;
	PriorityQueue m_closedQueue;

	void reset();
};