#pragma once

#include "GameEvent.h"
#include "Entity.h"
#include "EntityType.h"
#include "glm/glm.hpp"

class Projectile : public Entity
{
public:
	Projectile(const GameEvent& gameEvent);
	Projectile(Projectile&&) noexcept;
	Projectile& operator=(Projectile&&) noexcept;
	
	const GameEvent& getSenderEvent() const;
	bool isReachedDestination() const;

	void update(float deltaTime);

private:
	GameEvent m_gameEvent;
};