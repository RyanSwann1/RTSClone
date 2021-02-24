#pragma once

#include "GameEvents.h"
#include <queue>

class GameEventHandler
{
public:
	GameEventHandler(const GameEventHandler&) = delete;
	GameEventHandler& operator=(const GameEventHandler&) = delete;
	GameEventHandler(GameEventHandler&&) = delete;
	GameEventHandler& operator=(GameEventHandler&&) = delete;

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