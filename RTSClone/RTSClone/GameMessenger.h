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
void broadcastToMessenger(const Message& message)
{
	GameMessenger<Message, ID, ReturnType>::getInstance().broadcast(message);
}

template <typename Message, typename ID = int, typename ReturnType = void>
class GameMessengerSubscriber
{
public:
	GameMessengerSubscriber(const std::function<ReturnType(const Message&)>& callback)
		: id(GameMessenger<Message, ID, ReturnType>::getInstance().subscribe(callback))
	{}
	GameMessengerSubscriber(const ID id, const std::function<ReturnType(const Message&)>& callback)
		: id(GameMessenger<Message, ID, ReturnType>::getInstance().subscribe(id, callback))
	{}
	GameMessengerSubscriber(const GameMessengerSubscriber&) = delete;
	GameMessengerSubscriber& operator=(const GameMessengerSubscriber&) = delete;
	GameMessengerSubscriber(GameMessengerSubscriber&& rhs)
		: id(rhs.id)
	{
		rhs.active = false;
	}
	GameMessengerSubscriber& operator=(GameMessengerSubscriber&& rhs)
	{
		id = rhs.id;
		rhs.active = false;

		return *this;
	}
	~GameMessengerSubscriber()
	{
		if (!active)
		{
			GameMessenger<Message, ID, ReturnType>::getInstance().unsubscribe(id);
		}
	}

private:
	ID id;
	bool active = true;
};