#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <queue>

enum class eGameEventType
{
	Attack
};

struct GameEvent
{
	GameEvent(eGameEventType gameEventType, const glm::vec3& position, int senderID, int targetID);

	eGameEventType gameEventType;
	glm::vec3 position;
	int senderID;
	int targetID;
};

class GameEventHandler : private NonCopyable, private NonMovable
{
public:
	static GameEventHandler& getInstance()
	{
		static GameEventHandler instance;
		return instance;
	}
	
private:
	GameEventHandler();

	std::queue<GameEvent> m_gameEvents;
};