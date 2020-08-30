#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "GameEvent.h"
#include <queue>

class Map;
class ProjectileHandler;
class FactionPlayer;
class FactionAI;
class GameEventHandler : private NonCopyable, private NonMovable
{
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	void addEvent(const GameEvent& gameEvent);
	void handleEvents(FactionPlayer& player, FactionAI& playerAI, ProjectileHandler& projectileHandler, const Map& map);
	
private:
	GameEventHandler();

	std::queue<GameEvent> m_gameEvents;
};