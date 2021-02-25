#pragma once

class ActiveStatus
{
public:
	ActiveStatus()
		: active(true) {}
	ActiveStatus(const ActiveStatus&) = delete;
	ActiveStatus& operator=(const ActiveStatus&) = delete;
	ActiveStatus(ActiveStatus&& rhs) noexcept
		: active(rhs.active)
	{
		rhs.active = false;
	}
	ActiveStatus& operator=(ActiveStatus&& rhs) noexcept
	{
		active = rhs.active;
		rhs.active = false;

		return *this;
	}

	bool isActive() const
	{
		return active;
	}

private:
	bool active;
};