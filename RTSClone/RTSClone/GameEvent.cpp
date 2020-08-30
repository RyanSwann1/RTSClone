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
{
	assert(gameEventType == eGameEventType::RemoveAllWorkerPlannedBuildings ||
		gameEventType == eGameEventType::AddResources);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID),
	startingPosition(),
	endingPosition()
{
	assert(gameEventType == eGameEventType::Attack);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID, const glm::vec3 & startingPosition, const glm::vec3 & endingPosition)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetID(targetID),
	startingPosition(startingPosition),
	endingPosition(endingPosition)
{
	assert(gameEventType == eGameEventType::SpawnProjectile);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionName senderFaction, const glm::vec3 & position)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(Globals::INVALID_ENTITY_ID),
	targetID(Globals::INVALID_ENTITY_ID),
	startingPosition(position),
	endingPosition()
{
	assert(gameEventType == eGameEventType::RemovePlannedBuilding);
}