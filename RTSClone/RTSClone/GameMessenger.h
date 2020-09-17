#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameMessageType.h"
#include <functional>
#include <vector>
#include <assert.h>
#include <array>

struct Listener : private NonCopyable
{
	Listener(const std::function<void(const void*)>& fp, const void* ownerAddress);
	Listener(Listener&&) noexcept;
	Listener& operator=(Listener&&) noexcept;

	std::function<void(const void*)> m_listener;
	const void* m_ownerAddress;
};

class GameMessenger : private NonCopyable, private NonMovable
{
public:
	static GameMessenger& getInstance()
	{
		static GameMessenger instance;
		return instance;
	}

	template <typename GameMessage>
	void subscribe(const std::function<void(const GameMessage&)>& gameMessage, const void* ownerAddress)
	{
		auto& listeners = m_listeners[static_cast<int>(GameMessage::getType())];
		assert(!isOwnerAlreadyRegistered(listeners, ownerAddress));

		listeners.emplace_back(reinterpret_cast<std::function<void(const void*)> const&>(gameMessage), ownerAddress);
	}

	template <typename GameMessage>
	void unsubscribe(const void* ownerAddress)
	{
		auto& listeners = m_listeners[static_cast<int>(GameMessage::getType())];
		assert(isOwnerAlreadyRegistered(listeners, ownerAddress));

		auto iter = std::find_if(listeners.begin(), listeners.end(), [ownerAddress](const auto& listener)
		{
			return listener.m_ownerAddress == ownerAddress;
		});

		assert(iter != listeners.end());
		listeners.erase(iter);
	}

	template <typename GameMessage>
	void broadcast(GameMessage gameMessage)
	{
		const auto& listeners = m_listeners[static_cast<int>(GameMessage::getType())];
		assert(!listeners.empty());
		for (const auto& listener : listeners)
		{
			reinterpret_cast<std::function<void(const GameMessage&)> const&>(listener.m_listener)(gameMessage);
		}
	}

private:
	GameMessenger() {}
	std::array<std::vector<Listener>, static_cast<size_t>(eGameMessageType::Max) + 1> m_listeners;

	bool isOwnerAlreadyRegistered(const std::vector<Listener>& listeners, const void* ownerAddress) const;
};