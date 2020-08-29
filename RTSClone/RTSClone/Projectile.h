#pragma once

#include "GameEvent.h"
#include "Entity.h"
#include "EntityType.h"
#include "glm/glm.hpp"

struct Projectile : public Entity
{
	Projectile(const GameEvent& gameEvent);
	Projectile(Projectile&&) noexcept;
	Projectile& operator=(Projectile&&) noexcept;
	
	bool isReachedDestination() const;

	void update(float deltaTime);

	GameEvent m_gameEvent;
};