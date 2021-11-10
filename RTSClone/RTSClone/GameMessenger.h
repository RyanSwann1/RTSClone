#pragma once

#include <functional>
#include <vector>
#include <assert.h>
#include <algorithm>

template <typename Message>
class GameMessenger
{
	struct Listener
	{
		Listener(const std::function<void(const Message&)>& callback, int ID)
			: callback(callback),
			ID(ID)
		{}

		std::function<void(const Message&)> callback;
		int ID;
	};

public:
	GameMessenger(const GameMessenger&) = delete;
	GameMessenger& operator=(const GameMessenger&) = delete;
	GameMessenger(GameMessenger&&) = delete;
	GameMessenger& operator=(GameMessenger&&) = delete;

	static GameMessenger<Message>& getInstance()
	{
		static GameMessenger<Message> instance;
		return instance;
	}

	[[nodiscard]] int subscribe(const std::function<void(const Message&)>& callback)
	{
		int ID = m_uniqueID++;
		m_listeners.emplace_back(callback, ID);
		return ID;
	}

	void unsubscribe(int ID)
	{
		auto listener = std::find_if(m_listeners.begin(), m_listeners.end(), [ID](const auto& listener)
		{
			return listener.ID == ID;
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

	std::vector<Listener> m_listeners	= {};
	int m_uniqueID						= { 0 };
};

template <typename Message>
[[nodiscard]] int subscribeToMessenger(const std::function<void(const Message&)>& callback)
{
	return GameMessenger<Message>::getInstance().subscribe(callback);
}

template <typename Message>
void unsubscribeToMessenger(int ID)
{
	GameMessenger<Message>::getInstance().unsubscribe(ID);
}

template <typename Message>
void broadcastToMessenger(const Message& message)
{
	GameMessenger<Message>::getInstance().broadcast(message);
}