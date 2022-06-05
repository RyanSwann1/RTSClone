#include "UniqueID.h"
#include <utility>

namespace
{
	int unique_id = 0;
}

UniqueID::UniqueID()
	: m_id(++unique_id)
{}

UniqueID::UniqueID(UniqueID&& rhs) noexcept
	: m_id(rhs.m_id)
{
	rhs.m_id = INVALID_ID;
}

UniqueID& UniqueID::operator=(UniqueID&& rhs) noexcept
{
	std::swap(m_id, rhs.m_id);
	return *this;
}

int UniqueID::Get() const
{
	return m_id;
}