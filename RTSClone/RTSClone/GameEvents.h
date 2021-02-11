#pragma once

#include "FactionController.h"
#include "glm/glm.hpp"
#include "EntityType.h"

enum class eGameEventType
{
	TakeDamage = 0,
	SpawnProjectile,
	RevalidateMovementPaths,
	EliminateFaction,
	PlayerSpawnEntity,
	PlayerActivatePlannedBuilding,
	RepairEntity,
	SetTargetEntityGUI,
	ResetTargetEntityGUI,
	IncreaseFactionShield,
	OnEnteredIdleState,
	AttachFactionToBase
};

struct GameEvent_0
{
	GameEvent_0() {}
};
struct RevalidateMovementPathsEvent : public GameEvent_0 {};
struct ResetTargetEntityGUIEvent : public GameEvent_0 {};

struct GameEvent_1
{
	GameEvent_1(eEntityType entityType, int targetID);

	eEntityType entityType;
	int targetID;
};
struct PlayerSpawnEntity : public GameEvent_1 {
	PlayerSpawnEntity(eEntityType entityType, int targetID) :
		GameEvent_1(entityType, targetID) {}
};
struct PlayerActivatePlannedBuildingEvent : public GameEvent_1 {
	PlayerActivatePlannedBuildingEvent(eEntityType entityType, int targetID) :
		GameEvent_1(entityType, targetID) {}
};

struct GameEvent_2
{
	GameEvent_2(eFactionController senderFaction);

	eFactionController factionController;
};
struct EliminateFactionEvent : public GameEvent_2 {
	EliminateFactionEvent(eFactionController factionController) :
		GameEvent_2(factionController) {}
};
struct IncreaseFactionShieldEvent : public GameEvent_2
{
	IncreaseFactionShieldEvent(eFactionController factionController) :
		GameEvent_2(factionController) {}
};

struct GameEvent_3
{
	GameEvent_3(eFactionController senderFaction, int senderID);

	eFactionController factionController;
	int entityID;
};
struct RepairEntityEvent : public GameEvent_3 {
	RepairEntityEvent(eFactionController factionController, int senderID) :
		GameEvent_3(factionController, senderID) {}
};
struct SetTargetEntityGUIEvent : public GameEvent_3 {
	SetTargetEntityGUIEvent(eFactionController factionController, int senderID) :
		GameEvent_3(factionController, senderID) {}
};

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
struct TakeDamageEvent : public GameEvent_4 {
	TakeDamageEvent(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage) : GameEvent_4(senderFaction, senderID, targetFaction, targetID, damage) {}
};

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
struct SpawnProjectileEvent : public GameEvent_5 {
	SpawnProjectileEvent(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition)
		: GameEvent_5(senderFaction, senderID, targetFaction, targetID, damage, startingPosition, endingPosition) {}
};

struct GameEvent_6
{
	GameEvent_6(eFactionController factionController, eEntityType entityType, int entityID);

	eFactionController factionController;
	eEntityType entityType;
	int entityID;
};
struct OnEnteredIdleStateEvent : public GameEvent_6
{
	OnEnteredIdleStateEvent(eFactionController factionController, eEntityType entityType, int entityID) :
		GameEvent_6(factionController, entityType, entityID) {}
};

struct GameEvent_7
{
	GameEvent_7(eFactionController factionController, const glm::vec3& position);

	eFactionController factionController;
	glm::vec3 position;
};
struct AttachFactionToBaseEvent : public GameEvent_7
{
	AttachFactionToBaseEvent(eFactionController factionController, const glm::vec3& position)
		: GameEvent_7(factionController, position) {}
};

union GameEvents
{
	RevalidateMovementPathsEvent			revalidateMovementPaths;
	ResetTargetEntityGUIEvent				resetTargetEntityGUI;
	PlayerSpawnEntity						playerSpawnEntity;
	PlayerActivatePlannedBuildingEvent		playerActivatePlannedBuilding;
	EliminateFactionEvent					eliminateFaction;
	IncreaseFactionShieldEvent				increaseFactionShield;
	RepairEntityEvent						repairEntity;
	SetTargetEntityGUIEvent					setTargetEntityGUI;
	TakeDamageEvent							takeDamage;
	SpawnProjectileEvent					spawnProjectile;
	OnEnteredIdleStateEvent					onEnteredIdleState;
	AttachFactionToBaseEvent				attachFactionToBase;

	GameEvents(RevalidateMovementPathsEvent gameEvent) :			revalidateMovementPaths(gameEvent) {}
	GameEvents(ResetTargetEntityGUIEvent gameEvent) :				resetTargetEntityGUI(gameEvent) {}
	GameEvents(IncreaseFactionShieldEvent gameEvent) :				increaseFactionShield(gameEvent) {}
	GameEvents(PlayerSpawnEntity gameEvent) :						playerSpawnEntity(gameEvent) {}
	GameEvents(PlayerActivatePlannedBuildingEvent gameEvent) :		playerActivatePlannedBuilding(gameEvent) {}
	GameEvents(EliminateFactionEvent gameEvent) :					eliminateFaction(gameEvent) {}
	GameEvents(RepairEntityEvent gameEvent) :						repairEntity(gameEvent) {}
	GameEvents(SetTargetEntityGUIEvent gameEvent) :					setTargetEntityGUI(gameEvent) {}
	GameEvents(const TakeDamageEvent& gameEvent) :					takeDamage(gameEvent) {}
	GameEvents(const SpawnProjectileEvent& gameEvent) :				spawnProjectile(gameEvent) {}
	GameEvents(const OnEnteredIdleStateEvent& gameEvent) :			onEnteredIdleState(gameEvent) {}
	GameEvents(const AttachFactionToBaseEvent& gameEvent) :			attachFactionToBase(gameEvent) {}
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
	static GameEvent createEliminateFaction(eFactionController factionController);
	static GameEvent createIncreaseFactionShield(eFactionController factionController);
	
	//GameEvent_3
	static GameEvent createRepairEntity(eFactionController senderFaction, int senderID);
	static GameEvent createSetTargetEntityGUI(eFactionController senderFaction, int senderID);

	//GameEvent_4
	static GameEvent createTakeDamage(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage);

	//GameEvent_5
	static GameEvent createSpawnProjectile(eFactionController senderFaction, int senderID, eFactionController targetFaction,
		int targetID, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition);

	//GameEvent_6
	static GameEvent createOnEnteredIdleState(eFactionController factionController, eEntityType entityType, int targetID);

	//GameEvent_7
	static GameEvent createAttachFactionToBase(eFactionController factionController, const glm::vec3& position);

	eGameEventType type;
	GameEvents data;
};