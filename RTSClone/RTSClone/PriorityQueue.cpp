#include "PriorityQueue.h"

//PriorityQueueNode
PriorityQueueNode::PriorityQueueNode(const glm::ivec2& position, const glm::ivec2& parentPosition, float g, float h)
	: position(position),
	parentPosition(parentPosition),
	g(g),
	h(h)
{}

float PriorityQueueNode::getF() const
{
	return h + g;
}

//PriorityQueue
PriorityQueue::PriorityQueue(size_t size)
	: priority_queue(nodeCompare),
	m_addedNodePositions()
{
	c.reserve(size);
}

size_t PriorityQueue::getSize() const
{
	return c.size();
}

void PriorityQueue::clear()
{
	c.clear();
	m_addedNodePositions.clear();
}

bool PriorityQueue::isEmpty() const
{
	return c.empty();
}

void PriorityQueue::add(const PriorityQueueNode& node)
{
	assert(!contains(node.position));

	push(node);
	m_addedNodePositions.insert(node.position);
}

void PriorityQueue::popTop()
{
	assert(!isEmpty());

	auto iter = m_addedNodePositions.find(top().position);
	assert(iter != m_addedNodePositions.end());
	m_addedNodePositions.erase(iter);

	pop();
}

bool PriorityQueue::contains(const glm::ivec2& position) const
{
	return m_addedNodePositions.find(position) != m_addedNodePositions.cend();
}

const PriorityQueueNode& PriorityQueue::getTop() const
{
	assert(!isEmpty());
	return top();
}

PriorityQueueNode& PriorityQueue::getNode(const glm::ivec2& position)
{
	auto iter = std::find_if(c.begin(), c.end(), [&position](const auto& node) -> bool
	{
		return node.position == position;
	});

	assert(iter != c.end());
	return (*iter);
}

bool PriorityQueue::isSuccessorNodeValid(const PriorityQueueNode& successorNode) const
{
	auto matchingNode = std::find_if(c.cbegin(), c.cend(), [&successorNode](const auto& node) -> bool
	{
		return successorNode.position == node.position;
	});
	if (matchingNode != c.cend() && successorNode.getF() < matchingNode->getF())
	{
		return true;
	}

	return false;
}