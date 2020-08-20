#include "GameMessenger.h"

Listener::Listener(const std::function<void(const void*)>& fp, const void* ownerAddress)
	: m_listener(fp),
	m_ownerAddress(ownerAddress)
{
	assert(ownerAddress != nullptr);
}

Listener::Listener(Listener&& orig) noexcept
	: m_listener(orig.m_listener),
	m_ownerAddress(orig.m_ownerAddress)
{
	orig.m_listener = nullptr;
	orig.m_ownerAddress = nullptr;
}

Listener& Listener::operator=(Listener&& orig) noexcept
{
	m_listener = orig.m_listener;
	m_ownerAddress = orig.m_ownerAddress;

	orig.m_listener = nullptr;
	orig.m_ownerAddress = nullptr;

	return *this;
}

bool GameMessenger::isOwnerAlreadyRegistered(const std::vector<Listener>& listeners, eGameMessageType gameEventType, const void* ownerAddress) const
{
	assert(ownerAddress != nullptr);

	if (!listeners.empty())
	{
		auto result = std::find_if(listeners.cbegin(), listeners.cend(), [ownerAddress](const auto& listener)
		{
			return listener.m_ownerAddress == ownerAddress;
		});

		return result != listeners.cend();
	}
	else
	{
		return false;
	}
}