#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "GameEvent.h"
#include <queue>
#include <vector>
#include <memory>

class Level;
class Map;
class ProjectileHandler;
class FactionHandler;
class GameEventHandler : private NonCopyable, private NonMovable
{
	friend class Level;
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	void addEvent(const GameEvent& gameEvent);
	
private:
	GameEventHandler();

	std::queue<GameEvent> m_gameEvents;

	void handleEvents(Level& level, const Map& map);
};