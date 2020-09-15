#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "GameEvent.h"
#include <queue>
#include <vector>
#include <memory>

class Map;
class ProjectileHandler;
class Faction;
class GameEventHandler : private NonCopyable, private NonMovable
{
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}

	void addEvent(const GameEvent& gameEvent);
	void handleEvents(std::vector<std::unique_ptr<Faction>>& factions, ProjectileHandler& projectileHandler, const Map& map);
	
private:
	GameEventHandler();

	std::queue<GameEvent> m_gameEvents;
};