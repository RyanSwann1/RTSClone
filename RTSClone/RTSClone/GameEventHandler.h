#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <queue>

class FactionPlayer;
class FactionAI;
class Map;
class GameEventHandler : private NonCopyable, private NonMovable
{
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	void addEvent(const GameEvent& gameEvent);
	void handleEvents(FactionPlayer& player, FactionAI& playerAI, const Map& map);
	
private:
	GameEventHandler();

	std::queue<GameEvent> m_gameEvents;
};