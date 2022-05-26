#pragma once

#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"
#include "Events/GameMessenger.h"
#include <unordered_map>
#include <vector>

struct MinHeapNode
{
	MinHeapNode() = default;
	MinHeapNode(glm::ivec2 position, glm::ivec2 cameFrom, float g, float h);

	float getCost() const;

	glm::ivec2 position{ 0, 0 };
	glm::ivec2 cameFrom{ 0, 0 };
	float g{ 0.f };
	float h{ 0.f };
};

namespace GameMessages
{
	struct MapSize;
}
class MinHeap
{
public:
	MinHeap();
	MinHeap(const MinHeap&) = delete;
	MinHeap& operator=(const MinHeap&) = delete;
	MinHeap(MinHeap&&) = default;
	MinHeap& operator=(MinHeap&&) = default;

	bool isEmpty() const;

	void add(const MinHeapNode& node);
	MinHeapNode pop();
	bool findAndErase(glm::ivec2 position);
	void clear();

private:
	std::vector<MinHeapNode> m_heap;
	std::unordered_map<glm::ivec2, size_t> m_indexes;
	BroadcasterSub<GameMessages::MapSize> m_onNewMapSizeID;

	void sortNode(size_t i);
	void swap(size_t index1, size_t index2);
	void onNewMapSize(const GameMessages::MapSize& gameMessage);
};