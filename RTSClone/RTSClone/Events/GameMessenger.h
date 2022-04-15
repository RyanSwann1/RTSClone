#pragma once

#include "IDGenerator.h"
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
		Listener(std::function<void(Message&&)> callback, const int id)
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

	[[nodiscard]] int subscribe(std::function<void(Message&&)> callback)
	{	
		const int id = id_generator::gen();
		assert(!is_listener_registered(id));
		m_listeners.emplace_back(callback, id);
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

	bool is_listener_registered(const int id) const
	{
		return std::any_of(m_listeners.cbegin(), m_listeners.cend(), [id](auto listener)
		{
			return listener.id == id;
		});
	}

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
	BroadcasterSub(BroadcasterSub&& rhs) noexcept
		: id(rhs.id.swap(id))
	{}
	BroadcasterSub& operator=(BroadcasterSub&& rhs) noexcept
	{
		rhs.id.swap(id);
		return *this;
	}
	~BroadcasterSub()
	{
		if (id)
		{
			Broadcaster<Message>::getInstance().unsubscribe(*id);
		}
	}

private:
	std::optional<int> id = std::nullopt;
};