#pragma once

class Timer
{
public:
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
	float m_expirationTime;
	float m_elaspedTime;
	bool m_active;
};