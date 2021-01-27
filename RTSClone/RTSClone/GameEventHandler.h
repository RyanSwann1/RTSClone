#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameEvents.h"
#include <queue>

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