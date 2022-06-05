#pragma once

#include "Core/UniqueID.h"
#include <functional>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <optional>
#include <utility>

template <typename Message>
class Broadcaster
{
	struct Listener
	{
		Listener(const std::function<void(Message&&)> callback, const int id)
			: callback(callback),
			id(id)
		{}

		std::function<void(Message&&)> callback;
		int id;
	};

public:
	static Broadcaster<Message>& getInstance()
	{
		static Broadcaster<Message> instance;
		return instance;
	}

	[[nodiscard]] UniqueID subscribe(std::function<void(Message&&)> callback)
	{
		UniqueID id{};
		m_listeners.emplace_back(callback, id.Get());
		return id;
	}

	void unsubscribe(const int id)
	{
		const auto listener = std::find_if(m_listeners.cbegin(), m_listeners.cend(), [id](auto listener)
			{
				return listener.id == id;
			});
		assert(listener != m_listeners.cend());
		m_listeners.erase(listener);
	}

	void broadcast(Message&& message) const
	{
		assert(!m_listeners.empty());
		for (const auto& listener : m_listeners)
		{
			listener.callback(std::move(message));
		}
	}

private:
	Broadcaster() {}

	std::vector<Listener> m_listeners = {};
};

template <typename Message>
void broadcast(Message&& message)
{
	Broadcaster<Message>::getInstance().broadcast(std::forward<Message>(message));
}

template <typename Message>
class BroadcasterSub
{
public:
	BroadcasterSub(std::function<void(Message&&)> callback)
		: id(Broadcaster<Message>::getInstance().subscribe(callback))
	{}
	BroadcasterSub(const BroadcasterSub&) = delete;
	BroadcasterSub& operator=(const BroadcasterSub&) = delete;
	BroadcasterSub(BroadcasterSub&&) noexcept = default;
	BroadcasterSub& operator=(BroadcasterSub&&) noexcept = default;
	~BroadcasterSub()
	{
		if (id.Get() != UniqueID::INVALID_ID)
		{
			Broadcaster<Message>::getInstance().unsubscribe(id.Get());
		}
	}

private:
	UniqueID id{};
};