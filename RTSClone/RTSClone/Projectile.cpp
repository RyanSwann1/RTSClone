#include "Projectile.h"
#include "ShaderHandler.h"
#include "GameEvent.h"
#include "Globals.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 65.0f;
}

Projectile::Projectile(const GameEvent& gameEvent)
	: Entity(gameEvent.startingPosition, eEntityType::Projectile),
	m_gameEvent(gameEvent)
{}

Projectile::Projectile(Projectile&& orig) noexcept
	: Entity(std::move(orig)),
	m_gameEvent(orig.m_gameEvent)
{}

bool Projectile::isReachedDestination() const
{
	return m_position == m_gameEvent.endingPosition;
}

void Projectile::update(float deltaTime)
{
	m_position = Globals::moveTowards(m_position, m_gameEvent.endingPosition, MOVEMENT_SPEED * deltaTime);
}

Projectile& Projectile::operator=(Projectile&& orig) noexcept
{
	Entity::operator=(std::move(orig));
	m_gameEvent = orig.m_gameEvent;

	return *this;
}