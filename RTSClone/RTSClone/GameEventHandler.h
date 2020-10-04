#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameEvent.h"
#include <queue>
#include <memory>

class GameEventHandler : private NonCopyable, private NonMovable
{
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	std::queue<GameEvent> gameEvents;

private:
	GameEventHandler()
		: gameEvents()
	{}
};