#pragma once

#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include <unordered_map>
#include <vector>

struct MinHeapNode
{
	MinHeapNode();
	MinHeapNode(glm::ivec2 position, glm::ivec2 cameFrom, float g, float h);

	float getCost() const;

	glm::ivec2 position;
	glm::ivec2 cameFrom;
	float g;
	float h;
};

namespace GameMessages
{
	struct NewMapSize;
}
class MinHeap
{
public:
	MinHeap();
	MinHeap(const MinHeap&) = delete;
	MinHeap& operator=(const MinHeap&) = delete;
	MinHeap(MinHeap&&) = delete;
	MinHeap& operator=(MinHeap&&) = delete;
	~MinHeap();

	bool isEmpty() const;

	void add(const MinHeapNode& node);
	MinHeapNode pop();
	bool findAndErase(glm::ivec2 position);
	void clear();

private:
	std::vector<MinHeapNode> m_heap;
	std::unordered_map<glm::ivec2, size_t> m_indexes;

	void sortNode(size_t i);
	void swap(size_t index1, size_t index2);
	void onNewMapSize(const GameMessages::NewMapSize& gameMessage);
};