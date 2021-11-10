#pragma once

#include "GameMessenger.h"
#include <functional>

template <typename T>
class GameMessengerSubscriber
{ 
public:
	explicit GameMessengerSubscriber(const std::function<void(const T&)>& callback)
		: ID(subscribeToMessenger<T>(callback))
	{}
	GameMessengerSubscriber(const GameMessengerSubscriber&) = delete;
	GameMessengerSubscriber& operator=(const GameMessengerSubscriber&) = delete;
	GameMessengerSubscriber(GameMessengerSubscriber&& rhs)
		: ID(rhs.ID)
	{
		rhs.ID = INVALID_ID;
	}
	GameMessengerSubscriber& operator=(GameMessengerSubscriber&& rhs)
	{
		ID = rhs.ID;
		rhs.ID = INVALID_ID;

		return *this;
	}
	~GameMessengerSubscriber()
	{
		if (ID != INVALID_ID)
		{
			unsubscribeToMessenger<T>(ID);
		}
	}

private:
	int ID;
	const int INVALID_ID = -1;
};