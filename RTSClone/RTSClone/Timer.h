#pragma once

class Timer
{
public:
	Timer(float expirationTime, bool active);

	float getExpiredTime() const;
	float getElaspedTime() const;
	bool isExpired() const;

	void setActive(bool active);
	void update(float deltaTime);
	void resetElaspedTime();

private:
	float m_expirationTime;
	float m_elaspedTime;
	bool m_active;
};