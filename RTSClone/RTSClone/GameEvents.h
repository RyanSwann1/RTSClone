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
	ForceSelfDestructEntity,
	ResetTargetEntityGUI,
	IncreaseFactionShield,
	AttachFactionToBase,
	DetachFactionFromBase,
	EntityIdle,
	AddFactionResources
};

struct RevalidateMovementPathsEvent
{
	static const eGameEventType type = { eGameEventType::RevalidateMovementPaths };
};

struct ResetTargetEntityGUIEvent
{
	static const eGameEventType type = { eGameEventType::ResetTargetEntityGUI };
};

struct PlayerSpawnEntity
{
	static const eGameEventType type = { eGameEventType::PlayerSpawnEntity };
	eEntityType entityType;
	int targetID;
};

struct PlayerActivatePlannedBuildingEvent
{
	static const eGameEventType type = { eGameEventType::PlayerActivatePlannedBuilding };
	eEntityType entityType;
	int targetID;
};

struct EliminateFactionEvent 
{
	static const eGameEventType type = { eGameEventType::EliminateFaction };
	eFactionController factionController;
};

struct IncreaseFactionShieldEvent 
{
	static const eGameEventType type = { eGameEventType::IncreaseFactionShield };
	eFactionController factionController;
};

struct SelfDestructEntityEvent
{
	static const eGameEventType type = { eGameEventType::ForceSelfDestructEntity };
	eFactionController factionController;
	int entityID;
	eEntityType entityType;
};

struct SetTargetEntityGUIEvent
{
	static const eGameEventType type = { eGameEventType::SetTargetEntityGUI };
	eFactionController factionController;
	int entityID;
	eEntityType entityType;
};

struct RepairEntityEvent
{
	static const eGameEventType type = { eGameEventType::RepairEntity };
	eFactionController factionController;
	int entityID;
	eEntityType entityType;
};

struct TakeDamageEvent
{
	static const eGameEventType type = { eGameEventType::TakeDamage };
	eFactionController senderFaction;
	int senderID;
	eEntityType senderEntityType;
	eFactionController targetFaction;
	int targetID;
	int damage;
};

struct SpawnProjectileEvent
{
	static const eGameEventType type = { eGameEventType::SpawnProjectile };
	eFactionController senderFaction;
	int senderID;
	eEntityType senderEntityType;
	eFactionController targetFaction;
	int targetID;
	eEntityType targetEntityType;
	int damage;
	glm::vec3 spawnPosition;
	glm::vec3 destination;
};

struct AttachFactionToBaseEvent
{
	static const eGameEventType type = { eGameEventType::AttachFactionToBase };
	eFactionController factionController;
	glm::vec3 position;
};

struct DetachFactionFromBaseEvent
{
	static const eGameEventType type = { eGameEventType::DetachFactionFromBase };
	eFactionController factionController;
	glm::vec3 position;
};

struct EntityIdleEvent
{
	static const eGameEventType type = { eGameEventType::EntityIdle };
	int entityID = { 0 };
	eFactionController faction = eFactionController::None;
};

struct AddFactionResourcesEvent
{
	static const eGameEventType type = { eGameEventType::AddFactionResources };
	int quantity = { 0 };
	eFactionController faction = eFactionController::None;
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
	SelfDestructEntityEvent					forceSelfDestructEntity;
	TakeDamageEvent							takeDamage;
	SpawnProjectileEvent					spawnProjectile;
	AttachFactionToBaseEvent				attachFactionToBase;
	DetachFactionFromBaseEvent				detachFactionFromBase;
	EntityIdleEvent							entityIdle;
	AddFactionResourcesEvent				addFactionResources;

	GameEvents(RevalidateMovementPathsEvent gameEvent)			:	revalidateMovementPaths(gameEvent) {}
	GameEvents(ResetTargetEntityGUIEvent gameEvent)				:	resetTargetEntityGUI(gameEvent) {}
	GameEvents(IncreaseFactionShieldEvent gameEvent)			:	increaseFactionShield(gameEvent) {}
	GameEvents(PlayerSpawnEntity gameEvent)						:	playerSpawnEntity(gameEvent) {}
	GameEvents(PlayerActivatePlannedBuildingEvent gameEvent)	:	playerActivatePlannedBuilding(gameEvent) {}
	GameEvents(EliminateFactionEvent gameEvent)					:	eliminateFaction(gameEvent) {}
	GameEvents(const RepairEntityEvent& gameEvent)				:	repairEntity(gameEvent) {}
	GameEvents(const SetTargetEntityGUIEvent& gameEvent)		:	setTargetEntityGUI(gameEvent) {}
	GameEvents(const SelfDestructEntityEvent& gameEvent)		:	forceSelfDestructEntity(gameEvent) {}
	GameEvents(const TakeDamageEvent& gameEvent)				:   takeDamage(gameEvent) {}
	GameEvents(const SpawnProjectileEvent& gameEvent)			:	spawnProjectile(gameEvent) {}
	GameEvents(const AttachFactionToBaseEvent& gameEvent)		:   attachFactionToBase(gameEvent) {}
	GameEvents(const DetachFactionFromBaseEvent& gameEvent)		:	detachFactionFromBase(gameEvent) {}
	GameEvents(EntityIdleEvent gameEvent)						:   entityIdle(gameEvent) {}
	GameEvents(AddFactionResourcesEvent gameEvent)				:   addFactionResources(gameEvent) {}
};

struct GameEvent
{
	GameEvent() = delete;
	
	template <typename T>
	static GameEvent create(const T& gameEvent)
	{
		return { gameEvent.type, gameEvent };
	}

	eGameEventType type;
	GameEvents data;
};