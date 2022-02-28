#pragma once

#include <functional>
#include <vector>
#include <assert.h>
#include <algorithm>

template <typename Message, typename ID = int, typename ReturnType = void>
class GameMessenger
{
	struct Listener
	{
		Listener(const std::function<ReturnType(const Message&)>& callback, ID id)
			: callback(callback),
			id(id)
		{}

		std::function<ReturnType(const Message&)> callback;
		ID id;
	};

public:
	GameMessenger(const GameMessenger&) = delete;
	GameMessenger& operator=(const GameMessenger&) = delete;
	GameMessenger(GameMessenger&&) = delete;
	GameMessenger& operator=(GameMessenger&&) = delete;

	static GameMessenger<Message, ID, ReturnType>& getInstance()
	{
		static GameMessenger<Message, ID, ReturnType> instance;
		return instance;
	}

	[[nodiscard]] ID subscribe(const ID id, const std::function<ReturnType(const Message&)>& callback)
	{
		static_assert(!std::is_same<int, ID>());
		assert(!is_listener_registered(id));
		m_listeners.emplace_back(callback, id);

		return id;
	}

	[[nodiscard]] ID subscribe(const std::function<ReturnType(const Message&)>& callback)
	{
		static_assert(std::is_same<int, ID>());
		ID id = m_uniqueID++;
		assert(!is_listener_registered(id));
		m_listeners.emplace_back(callback, id);
		return id;
	}

	void unsubscribe(ID id)
	{
		auto listener = std::find_if(m_listeners.begin(), m_listeners.end(), [id](const auto& listener)
			{
				return listener.id == id;
			});
		assert(listener != m_listeners.cend());
		m_listeners.erase(listener);
	}

	ReturnType broadcast(ID id, const Message& message) const
	{
		static_assert(!std::is_same<int, ID>());
		assert(!m_listeners.empty());
		auto listener = std::find_if(m_listeners.cbegin(), m_listeners.cend(), [id](const auto& listener)
			{
				return listener.id == id;
			});
		assert(listener != m_listeners.cend());
		return listener->callback(message);
	}

	void broadcast(const Message& message) const
	{
		static_assert(std::is_same<int, ID>());
		assert(!m_listeners.empty());
		for (const auto& listener : m_listeners)
		{
			listener.callback(message);
		}
	}

private:
	GameMessenger() {}

	bool is_listener_registered(const ID id) const
	{
		return std::any_of(m_listeners.cbegin(), m_listeners.cend(), [id](const auto& listener)
		{
			return listener.id == id;
		});
	}

	std::vector<Listener> m_listeners	= {};
	int m_uniqueID						= { 0 };
};

template <typename Message, typename ID = int, typename ReturnType = void>
[[nodiscard]] ID subscribeToMessenger(const std::function<ReturnType(const Message&)>& callback)
{
	return GameMessenger<Message, ID, ReturnType>::getInstance().subscribe(callback);
}

template <typename Message, typename ID = int, typename ReturnType = void>
[[nodiscard]] ID subscribeToMessenger(const ID id, const std::function<ReturnType(const Message&)>& callback)
{
	return GameMessenger<Message, ID, ReturnType>::getInstance().subscribe(id, callback);
}

template <typename Message, typename ID = int, typename ReturnType = void>
void unsubscribeToMessenger(ID id)
{
	GameMessenger<Message, ID, ReturnType>::getInstance().unsubscribe(id);
}

template <typename Message, typename ID = int, typename ReturnType = void>
void broadcastToMessenger(const Message& message)
{
	GameMessenger<Message, ID, ReturnType>::getInstance().broadcast(message);
}