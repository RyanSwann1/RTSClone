#include "GameEvent.h"
#include "Globals.h"

//GameEvent
GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(Globals::INVALID_ENTITY_ID),
	startingPosition(),
	endingPosition()
{}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID),
	startingPosition(),
	endingPosition()
{}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID, const glm::vec3 & startingPosition, const glm::vec3 & endingPosition)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID),
	startingPosition(startingPosition),
	endingPosition(endingPosition)
{}