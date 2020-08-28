#include "GameEvent.h"
#include "Globals.h"

//GameEvent
GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(Globals::INVALID_ENTITY_ID)
{}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID)
{}