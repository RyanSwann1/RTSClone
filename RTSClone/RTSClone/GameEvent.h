#pragma once

#include "FactionName.h"
#include "glm/glm.hpp"

enum class eGameEventType
{
	Attack = 0,
	RemovePlannedBuilding,
	AddResources,
	SpawnProjectile
};

struct GameEvent
{
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID);
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID);
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, int senderID, int targetID,
		const glm::vec3& startingPosition, const glm::vec3& endingPosition);
	GameEvent(eGameEventType gameEventType, eFactionName senderFaction, const glm::vec3& position);

	eGameEventType type;
	eFactionName senderFaction;
	int senderID;
	int targetID;
	glm::vec3 startingPosition;
	glm::vec3 endingPosition;
};