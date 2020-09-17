#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"

enum class eGameEventType
{
	Attack = 0,
	RemovePlannedBuilding,
	RemoveAllWorkerPlannedBuildings,
	AddResources,
	SpawnProjectile,
	RevalidateMovementPaths,
	FactionEliminated
};

struct GameEvent
{
	GameEvent(eGameEventType gameEventType);
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
};