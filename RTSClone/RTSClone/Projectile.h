#pragma once

#include "GameEvent.h"
#include "AABB.h"
#include <functional>

struct Model;
class ShaderHandler;
class Projectile
{
public:
	Projectile(const SpawnProjectileEvent& gameEvent);
	
	const AABB& getAABB() const;
	const SpawnProjectileEvent& getSenderEvent() const;
	bool isReachedDestination() const;

	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler) const;

private:
	SpawnProjectileEvent m_senderEvent;
	glm::vec3 m_position;
	AABB m_AABB;
	std::reference_wrapper<const Model> m_model;
};