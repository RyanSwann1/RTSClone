#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "glm/glm.hpp"
#include <vector>
#include <queue>
#include <unordered_set>

//Pathfinding Optimisations
//https://www.reddit.com/r/gamedev/comments/dk19g6/new_pathfinding_algorithm_factorio/

struct PriorityQueueNode
{
	PriorityQueueNode(const glm::ivec2& position, const glm::ivec2& parentPosition, float g, float h);

	float getF() const;

	glm::ivec2 position;
	glm::ivec2 parentPosition;
	float g; 
	float h; 
};

const auto nodeCompare = [](const auto& a, const auto& b) -> bool { return b.getF() < a.getF(); };
class PriorityQueue : private std::priority_queue<PriorityQueueNode, std::vector<PriorityQueueNode>, decltype(nodeCompare)>
{
public:
	PriorityQueue();

	size_t getSize() const;
	bool isEmpty() const;
	bool contains(const glm::ivec2& position) const;
	const PriorityQueueNode& getTop() const;
	const PriorityQueueNode& getNode(const glm::ivec2& position) const;
	bool isSuccessorNodeValid(const PriorityQueueNode& successorNode) const;

	void setSize(const glm::ivec2& mapSize);
	void changeNode(const PriorityQueueNode& newNode);
	void add(const PriorityQueueNode& node);
	void popTop();
	void clear();

private:
	size_t m_maxSize;
	std::unordered_set<glm::ivec2> m_addedNodePositions;

	void eraseNode(const glm::ivec2& position);
};