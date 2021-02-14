#include "Projectile.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "ModelManager.h"
#include "Model.h"

namespace
{
	const float MOVEMENT_SPEED = 100.0f;
}

Projectile::Projectile(const SpawnProjectileEvent& gameEvent)
	: m_senderEvent(gameEvent),
	m_position(gameEvent.spawnPosition),
	m_AABB(m_position, ModelManager::getInstance().getModel(PROJECTILE_MODEL_NAME)),
	m_model(ModelManager::getInstance().getModel(PROJECTILE_MODEL_NAME))
{}

const AABB& Projectile::getAABB() const
{
	return m_AABB;
}

const SpawnProjectileEvent& Projectile::getSenderEvent() const
{
	return m_senderEvent;
}

bool Projectile::isReachedDestination() const
{
	return m_position == m_senderEvent.destination;
}

void Projectile::update(float deltaTime)
{
	m_position = Globals::moveTowards(m_position, m_senderEvent.destination, MOVEMENT_SPEED * deltaTime);
	m_AABB.update(m_position);
}

void Projectile::render(ShaderHandler& shaderHandler) const
{
	m_model.get().render(shaderHandler, m_position);
}