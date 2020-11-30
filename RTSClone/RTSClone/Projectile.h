#pragma once

#include "GameEvent.h"
#include "Entity.h"
#include "EntityType.h"
#include "glm/glm.hpp"

class Projectile : public Entity
{
public:
	Projectile(const GameEvent_5& gameEvent);
	Projectile(Projectile&&) noexcept;
	Projectile& operator=(Projectile&&) noexcept;
	
	const GameEvent_5& getSenderEvent() const;
	bool isReachedDestination() const;

	void update(float deltaTime);

private:
	GameEvent_5 m_senderEvent;
};