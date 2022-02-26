#pragma once

#include "GameMessenger.h"

template <typename Message, typename ID = int, typename ReturnType = void>
class GameMessengerSubscriber
{ 
public:
	GameMessengerSubscriber(const std::function<ReturnType(const Message&)>& callback)
		: id(subscribeToMessenger<Message, ID, ReturnType>(callback))
	{}
	GameMessengerSubscriber(const ID id, const std::function<ReturnType(const Message&)>& callback)
		: id(subscribeToMessenger<Message, ID, ReturnType>(id, callback))
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
			unsubscribeToMessenger<Message, ID, ReturnType>(id);
		}
	}

private:
	ID id;
	bool active = true;
};