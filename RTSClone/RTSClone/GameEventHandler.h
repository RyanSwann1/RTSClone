#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include "FactionName.h"
#include <queue>

enum class eGameEventType
{
	Attack = 0,
	RemovePlannedBuilding,
	AddResources
};

struct GameEvent
{
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID);
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID);

	const eGameEventType type;
	const eFactionName senderFaction;
	const int senderID;
	const int targetID;
};

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