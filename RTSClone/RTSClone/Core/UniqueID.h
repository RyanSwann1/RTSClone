#pragma once

class UniqueID
{
	static constexpr int INVALID_ID = -1;
public:
	UniqueID();
	UniqueID(const UniqueID&) = delete;
	UniqueID& operator=(const UniqueID&) = delete;
	UniqueID(UniqueID&&) noexcept;
	UniqueID& operator=(UniqueID&&) noexcept;

	int Get() const;

private:
	int m_id{ INVALID_ID };
};