#pragma once

#include "GameEvent.h"
#include "Entity.h"
#include "EntityType.h"
#include "glm/glm.hpp"

class Projectile : public Entity
{
public:
	Projectile(const SpawnProjectileEvent& gameEvent);
	Projectile(Projectile&&) noexcept;
	Projectile& operator=(Projectile&&) noexcept;
	
	const SpawnProjectileEvent& getSenderEvent() const;
	bool isReachedDestination() const;

	void update(float deltaTime);

private:
	SpawnProjectileEvent m_senderEvent;
};