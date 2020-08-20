#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameEventType.h"
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

	template <typename GameEvent>
	void subscribe(const std::function<void(const GameEvent&)>& gameEvent, const void* ownerAddress)
	{
		auto& listeners = m_listeners[static_cast<int>(GameEvent::getType())];
		assert(!isOwnerAlreadyRegistered(listeners, GameEvent::getType(), ownerAddress));

		listeners.emplace_back(reinterpret_cast<std::function<void(const void*)> const&>(gameEvent), ownerAddress);
	}

	template <typename GameEvent>
	void unsubscribe(const void* ownerAddress)
	{
		auto& listeners = m_listeners[static_cast<int>(GameEvent::getType())];
		assert(isOwnerAlreadyRegistered(listeners, GameEvent::getType(), ownerAddress));

		auto iter = std::find_if(listeners.begin(), listeners.end(), [ownerAddress](const auto& listener)
		{
			return listener.m_ownerAddress == ownerAddress;
		});

		assert(iter != listeners.end());
		listeners.erase(iter);
	}

	template <typename GameEvent>
	void broadcast(GameEvent gameEvent)
	{
		const auto& listeners = m_listeners[static_cast<int>(GameEvent::getType())];
		for (const auto& listener : listeners)
		{
			reinterpret_cast<std::function<void(const GameEvent&)> const&>(listener.m_listener)(gameEvent);
		}
	}

private:
	GameMessenger() {}
	std::array<std::vector<Listener>, static_cast<size_t>(eGameEventType::Max) + 1> m_listeners;

	bool isOwnerAlreadyRegistered(const std::vector<Listener>& listeners, eGameEventType gameEventType, const void* ownerAddress) const;
};