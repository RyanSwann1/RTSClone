#include "Core/Timer.h"
#include <assert.h>

Timer::Timer(float expirationTime, bool active)
	: m_expirationTime(expirationTime),
	m_elaspedTime(0.0f),
	m_active(active)
{}

float Timer::getExpiredTime() const
{
	return m_expirationTime;
}

float Timer::getElaspedTime() const
{
	return m_elaspedTime;
}

bool Timer::isExpired() const
{
	return m_elaspedTime >= m_expirationTime;
}

bool Timer::isActive() const
{
	return m_active;
}

void Timer::setExpirationTime(float expirationTime)
{
	assert(m_elaspedTime == 0.0f || isExpired());
	m_expirationTime = expirationTime;
}

void Timer::setActive(bool active)
{
	m_active = active;
}

void Timer::update(float deltaTime)
{
	if (m_active)
	{
		m_elaspedTime += deltaTime;
	}
}

void Timer::resetElaspedTime()
{
	m_elaspedTime = 0.0f;
}

void Timer::resetExpirationTime(float newExpirationTime)
{
	m_expirationTime = newExpirationTime;
}
