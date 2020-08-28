#pragma once

#include "FactionName.h"

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