#include "GameEvent.h"
#include "Globals.h"

GameEvent::GameEvent(eGameEventType gameEventType)
	: type(gameEventType),
	senderFaction(),
	senderID(Globals::INVALID_ENTITY_ID),
	targetFaction(),
	targetID(Globals::INVALID_ENTITY_ID),
	startingPosition(),
	endingPosition()
{
	assert(gameEventType == eGameEventType::RevalidateMovementPaths);
}

//GameEvent
GameEvent::GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetFaction(),
	targetID(Globals::INVALID_ENTITY_ID),
	startingPosition(),
	endingPosition()
{
	assert(gameEventType == eGameEventType::RemoveAllWorkerPlannedBuildings ||
		gameEventType == eGameEventType::AddResources);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID, 
	eFactionController targetFaction, int targetID)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetFaction(targetFaction),
	targetID(targetID),
	startingPosition(),
	endingPosition()
{
	assert(gameEventType == eGameEventType::Attack);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID, eFactionController targetFaction, int targetID,
	 const glm::vec3 & startingPosition, const glm::vec3 & endingPosition)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(senderID),
	targetFaction(targetFaction),
	targetID(targetID),
	startingPosition(startingPosition),
	endingPosition(endingPosition)
{
	assert(gameEventType == eGameEventType::SpawnProjectile);
}

GameEvent::GameEvent(eGameEventType gameEventType, eFactionController senderFaction, const glm::vec3 & position)
	: type(gameEventType),
	senderFaction(senderFaction),
	senderID(Globals::INVALID_ENTITY_ID),
	targetFaction(),
	targetID(Globals::INVALID_ENTITY_ID),
	startingPosition(position),
	endingPosition()
{
	assert(gameEventType == eGameEventType::RemovePlannedBuilding);
}