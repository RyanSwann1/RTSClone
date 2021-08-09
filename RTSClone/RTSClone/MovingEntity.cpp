#include "MovingEntity.h"

void MovingEntity::addToDestinationQueue(const glm::vec3& position)
{	
	auto cIter = std::find_if(m_destinationQueue.cbegin(), m_destinationQueue.cend(), [&position](const auto& existingPosition)
	{
		return existingPosition == position;
	});
	if (cIter == m_destinationQueue.cend())
	{
		m_destinationQueue.push_back(position);
	}
}

void MovingEntity::clearDestinationQueue()
{
	m_destinationQueue.clear();
}

MovingEntity::MovingEntity(const Model& model, const glm::vec3& startingPosition, eEntityType entityType, 
	int health, int shield, glm::vec3 startingRotation /*= glm::vec3(0.0f)*/)
	: Entity(model, startingPosition, entityType, health, shield, startingRotation)
{}