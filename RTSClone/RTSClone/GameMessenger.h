#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include <functional>
#include <vector>
#include <assert.h>

template <typename Message>
class GameMessenger : private NonCopyable, private NonMovable
{
	struct Listener
	{
		Listener(const std::function<void(const Message&)>& callback, const void* ownerAddress)
			: callback(callback),
			ownerAddress(ownerAddress)
		{}

		std::function<void(const Message&)> callback;
		const void* ownerAddress;
	};

public:
	static GameMessenger<Message>& getInstance()
	{
		static GameMessenger<Message> instance;
		return instance;
	}

	void subscribe(const std::function<void(const Message&)>& callback, const void* ownerAddress)
	{
		assert(!isRegistered(ownerAddress));
		m_listeners.emplace_back(callback, ownerAddress);
	}

	void unsubscribe(const void* ownerAddress)
	{
		assert(isRegistered(ownerAddress));
		auto listener = std::find_if(m_listeners.begin(), m_listeners.end(), [ownerAddress](const auto& listener)
		{
			return listener.ownerAddress == ownerAddress;
		});
		assert(listener != m_listeners.cend());
		m_listeners.erase(listener);
	}

	void broadcast(const Message& message) const
	{
		assert(!m_listeners.empty());
		for (const auto& listener : m_listeners)
		{
			listener.callback(message);
		}
	}

private:
	GameMessenger() {}

	std::vector<Listener> m_listeners;

	bool isRegistered(const void* ownerAddress) const
	{
		auto listener = std::find_if(m_listeners.cbegin(), m_listeners.cend(), [ownerAddress](const auto& listener)
		{
			return listener.ownerAddress == ownerAddress;
		});

		return listener != m_listeners.cend();
	}
};

template <typename Message>
void subscribeToMessenger(const std::function<void(const Message&)>& callback, const void* ownerAddress)
{
	GameMessenger<Message>::getInstance().subscribe(callback, ownerAddress);
}

template <typename Message>
void unsubscribeToMessenger(const void* ownerAddress)
{
	GameMessenger<Message>::getInstance().unsubscribe(ownerAddress);
}

template <typename Message>
void broadcastToMessenger(const Message& message)
{
	GameMessenger<Message>::getInstance().broadcast(message);
}