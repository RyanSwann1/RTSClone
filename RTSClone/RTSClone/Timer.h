#pragma once

class Timer
{
public:
	Timer() = default;
	Timer(float expirationTime, bool active);

	float getExpiredTime() const;
	float getElaspedTime() const;
	bool isExpired() const;
	bool isActive() const;

	void setExpirationTime(float expirationTime); 
	void setActive(bool active);
	void update(float deltaTime);
	void resetElaspedTime();
	void resetExpirationTime(float newExpirationTime);

private:
	float m_expirationTime	= 0.f;
	float m_elaspedTime		= 0.f;
	bool m_active			= false;
};