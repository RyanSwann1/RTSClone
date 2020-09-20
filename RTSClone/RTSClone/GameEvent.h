#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"
#include "EntityType.h"

//you can create an std::variant of different event types, or just use a discriminated union
//thing is you have to split different event types into structs

enum class eGameEventType
{
	Attack = 0,
	RemovePlannedBuilding,
	RemoveAllWorkerPlannedBuildings,
	AddResources,
	SpawnProjectile,
	RevalidateMovementPaths,
	FactionEliminated,
	SpawnUnit
};

struct GameEvent
{
	GameEvent(eGameEventType gameEventType);
	GameEvent(eGameEventType gameEventType, eEntityType entityType, int targetID);
	GameEvent(eGameEventType gameEventType, eFactionController senderFaction);
	GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID);
	GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID, 
		eFactionController targetFaction, int targetID, int damage);
	GameEvent(eGameEventType gameEventType, eFactionController senderFaction, int senderID, 
		eFactionController targetFaction, int targetID, int damage,
		const glm::vec3& startingPosition, const glm::vec3& endingPosition);
	GameEvent(eGameEventType gameEventType, eFactionController senderFaction, const glm::vec3& position);

	eGameEventType type;
	eFactionController senderFaction;
	int senderID;
	eFactionController targetFaction;
	int targetID;
	int damage;
	glm::vec3 startingPosition;
	glm::vec3 endingPosition;
	eEntityType entityType;
};