#include "Timer.h"

Timer::Timer(float expirationTime)
	: m_expirationTime(expirationTime),
	m_elaspedTime(0.0f),
	m_active(false)
{}

bool Timer::isExpired() const
{
	return m_elaspedTime >= m_expirationTime;
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