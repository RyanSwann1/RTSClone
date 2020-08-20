#include "GameEventHandler.h"
#include "Globals.h"

GameEvent::GameEvent(eGameEventType gameEventType, const glm::vec3& position, int senderID, int targetID)
	: gameEventType(gameEventType),
	position(position),
	senderID(senderID),
	targetID(targetID)
{}
