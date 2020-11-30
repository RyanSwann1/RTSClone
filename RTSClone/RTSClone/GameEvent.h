#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"
#include "EntityType.h"

enum class eGameEventType
{
	TakeDamage = 0,
	RemovePlannedBuilding,
	RemoveAllWorkerPlannedBuildings,
	AddResources,
	SpawnProjectile,
	RevalidateMovementPaths,
	EliminateFaction,
	PlayerSpawnUnit,
	PlayerActivatePlannedBuilding,
	RepairEntity,
	SetTargetEntityGUI,
	ResetTargetEntityGUI
};

struct GameEvent_0
{
	GameEvent_0() {}
};
using RevalidateMovementPathsEvent = GameEvent_0;
using ResetTargetEntityGUIEvent = GameEvent_0;

struct GameEvent_1
{
	GameEvent_1(eEntityType entityType, int targetID);

	eEntityType entityType;
	int targetID;
};
using PlayerSpawnUnitEvent = GameEvent_1;
using PlayerActivatePlannedBuildingEvent = GameEvent_1;

struct GameEvent_2
{
	GameEvent_2(eFactionController senderFaction);

	eFactionController factionController;
};
using EliminateFactionEvent = GameEvent_2;

struct GameEvent_3
{
	GameEvent_3(eFactionController senderFaction, int senderID);

	eFactionController factionController;
	int entityID;
};
using RemoveAllWorkerPlannedBuildingsEvent = GameEvent_3;
using AddResourcesEvent = GameEvent_3;
using RepairEntityEvent = GameEvent_3;
using SetTargetEntityGUIEvent = GameEvent_3;

struct GameEvent_4
{
	GameEvent_4(eFactionController senderFaction, int senderID, eFactionController targetFaction, 
		int targetID, int damage);

	eFactionController senderFaction;
	int senderID;
	eFactionController targetFaction;
	int targetID;
	int damage;
};
using TakeDamageEvent = GameEvent_4;

struct GameEvent_5
{
	GameEvent_5(eFactionController senderFaction, int senderID, eFactionController targetFaction, 
		int targetID, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition);

	eFactionController senderFaction;
	int senderID;
	eFactionController targetFaction;
	int targetID;
	int damage;
	glm::vec3 spawnPosition;
	glm::vec3 destination;
};
using SpawnProjectileEvent = GameEvent_5;

struct GameEvent_6
{
	GameEvent_6(eFactionController senderFaction, const glm::vec3& position);

	eFactionController factionController;
	glm::vec3 position;
};
using RemovePlannedBuildingEvent = GameEvent_6;

union GameEvents
{
	RevalidateMovementPathsEvent			revalidateMovementPaths;
	ResetTargetEntityGUIEvent				resetTargetEntityGUI;
	PlayerSpawnUnitEvent					playerSpawnUnit;
	PlayerActivatePlannedBuildingEvent		playerActivatePlannedBuilding;
	EliminateFactionEvent					eliminateFaction;
	RemoveAllWorkerPlannedBuildingsEvent	removeAllWorkerPlannedBuilding;
	AddResourcesEvent						addResources;
	RepairEntityEvent						repairEntity;
	SetTargetEntityGUIEvent					setTargetEntityGUI;
	TakeDamageEvent							takeDamage;
	SpawnProjectileEvent					spawnProjectile;
	RemovePlannedBuildingEvent				removePlannedBuilding;

	GameEvents(GameEvent_0 gameEvent)
	{
		revalidateMovementPaths = gameEvent;
		resetTargetEntityGUI = gameEvent;
	}
	GameEvents(GameEvent_1 gameEvent)
	{
		playerSpawnUnit = gameEvent;
		playerActivatePlannedBuilding = gameEvent;
	}
	GameEvents(GameEvent_2 gameEvent) : eliminateFaction(gameEvent) {}
	GameEvents(GameEvent_3 gameEvent)
	{
		removeAllWorkerPlannedBuilding = gameEvent;
		addResources = gameEvent;
		repairEntity = gameEvent;
		setTargetEntityGUI = gameEvent;
	}
	GameEvents(const GameEvent_4& gameEvent) : takeDamage(gameEvent) {}
	GameEvents(const GameEvent_5& gameEvent) : spawnProjectile(gameEvent) {}
	GameEvents(const GameEvent_6& gameEvent) : removePlannedBuilding(gameEvent) {}
};

struct GameEvent
{
	GameEvent() = delete;
	
	//GameEvent_0
	static GameEvent createRevalidateMovementPaths();
	static GameEvent createResetTargetEntityGUI();

	//GameEvent_1
	static GameEvent createPlayerSpawnUnit(eEntityType entityType, int targetID);
	static GameEvent createPlayerActivatePlannedBuilding(eEntityType, int targetID);
	
	//GameEvent_2
	static GameEvent createEliminateFaction(eFactionController senderFaction);
	
	//GameEvent_3
	static GameEvent createRemoveAllWorkerPlannedBuildings(eFactionController senderFaction, int senderID);
	static GameEvent createAddResources(eFactionController senderFaction, int senderID);
	static GameEvent createRepairEntity(eFactionController senderFaction, int senderID);
	static GameEvent createSetTargetEntityGUI(eFactionController senderFaction, int senderID);

	//GameEvent_4
	static GameEvent createTakeDamage(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage);

	//GameEvent_5
	static GameEvent createSpawnProjectile(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition);

	//GameEvent_6
	static GameEvent createRemovePlannedBuilding(eFactionController senderFaction, const glm::vec3& position);

	eGameEventType type;
	GameEvents data;
};