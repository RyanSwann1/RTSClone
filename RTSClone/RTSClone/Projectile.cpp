#include "Projectile.h"
#include "ShaderHandler.h"
#include "GameEvent.h"
#include "Globals.h"
#include "ModelManager.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 90.0f;
}

Projectile::Projectile(const GameEvent_5& gameEvent)
	: Entity(ModelManager::getInstance().getModel(PROJECTILE_MODEL_NAME), gameEvent.startingPosition, eEntityType::Projectile),
	m_senderEvent(gameEvent)
{}

Projectile::Projectile(Projectile&& orig) noexcept
	: Entity(std::move(orig)),
	m_senderEvent(orig.m_senderEvent)
{}

const GameEvent_5& Projectile::getSenderEvent() const
{
	return m_senderEvent;
}

bool Projectile::isReachedDestination() const
{
	return m_position == m_senderEvent.endingPosition;
}

void Projectile::update(float deltaTime)
{
	m_position = Globals::moveTowards(m_position, m_senderEvent.endingPosition, MOVEMENT_SPEED * deltaTime);
	m_AABB.update(m_position);
}

Projectile& Projectile::operator=(Projectile&& orig) noexcept
{
	Entity::operator=(std::move(orig));
	m_senderEvent = orig.m_senderEvent;

	return *this;
}