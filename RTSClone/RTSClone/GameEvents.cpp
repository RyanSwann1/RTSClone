#include "GameEvents.h"
#include "Globals.h"

GameEvent_1::GameEvent_1(eEntityType entityType, int targetID)
	: entityType(entityType),
	targetID(targetID)
{}

GameEvent_2::GameEvent_2(eFactionController senderFaction)
	: factionController(senderFaction)
{}

GameEvent_3::GameEvent_3(eFactionController senderFaction, int senderID)
	: factionController(senderFaction),
	entityID(senderID)
{}

GameEvent_4::GameEvent_4(eFactionController senderFaction, int senderID, eFactionController targetFaction, int targetID, int damage)
	: senderFaction(senderFaction),
	senderID(senderID),
	targetFaction(targetFaction),
	targetID(targetID),
	damage(damage)
{}

GameEvent_5::GameEvent_5(eFactionController senderFaction, int senderID, eFactionController targetFaction, int targetID, int damage, 
	const glm::vec3& startingPosition, const glm::vec3& endingPosition)
	: senderFaction(senderFaction),
	senderID(senderID),
	targetFaction(targetFaction),
	targetID(targetID),
	damage(damage),
	spawnPosition(startingPosition),
	destination(endingPosition)
{}

GameEvent_6::GameEvent_6(eFactionController factionController, eEntityType entityType, int entityID)
	: factionController(factionController),
	entityType(entityType),
	entityID(entityID)
{}

GameEvent_7::GameEvent_7(eFactionController factionController, const glm::vec3& position)
	: factionController(factionController),
	position(position)
{}

GameEvent GameEvent::createRevalidateMovementPaths()
{
	return { eGameEventType::RevalidateMovementPaths, RevalidateMovementPathsEvent{} };
}

GameEvent GameEvent::createResetTargetEntityGUI()
{
	return { eGameEventType::ResetTargetEntityGUI, ResetTargetEntityGUIEvent{} };
}

GameEvent GameEvent::createPlayerSpawnUnit(eEntityType entityType, int targetID)
{
	return { eGameEventType::PlayerSpawnEntity, PlayerSpawnEntity{entityType, targetID} };
}

GameEvent GameEvent::createPlayerActivatePlannedBuilding(eEntityType entityType, int targetID)
{
	return { eGameEventType::PlayerActivatePlannedBuilding, PlayerActivatePlannedBuildingEvent{entityType, targetID} };
}

GameEvent GameEvent::createOnEnteredIdleState(eFactionController factionController, eEntityType entityType, int targetID)
{
	return { eGameEventType::OnEnteredIdleState, OnEnteredIdleStateEvent{factionController, entityType, targetID} };
}

GameEvent GameEvent::createAttachFactionToBase(eFactionController factionController, const glm::vec3& position)
{
	return { eGameEventType::AttachFactionToBase, AttachFactionToBaseEvent{factionController, position} };
}

GameEvent GameEvent::createEliminateFaction(eFactionController factionController)
{
	return { eGameEventType::EliminateFaction, EliminateFactionEvent{factionController} };
}

GameEvent GameEvent::createIncreaseFactionShield(eFactionController factionController)
{
	return { eGameEventType::IncreaseFactionShield, IncreaseFactionShieldEvent{factionController} };	
}

GameEvent GameEvent::createRepairEntity(eFactionController senderFaction, int senderID)
{
	return { eGameEventType::RepairEntity, RepairEntityEvent{senderFaction, senderID} };
}

GameEvent GameEvent::createSetTargetEntityGUI(eFactionController senderFaction, int senderID)
{
	return { eGameEventType::SetTargetEntityGUI,SetTargetEntityGUIEvent{senderFaction, senderID} };
}

GameEvent GameEvent::createTakeDamage(eFactionController senderFaction, int senderID, eFactionController targetFaction, 
	int targetID, int damage)
{
	return { eGameEventType::TakeDamage, TakeDamageEvent{senderFaction, senderID, targetFaction, targetID, damage} };
}

GameEvent GameEvent::createSpawnProjectile(eFactionController senderFaction, int senderID, eFactionController targetFaction, 
	int targetID, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition)
{
	return { eGameEventType::SpawnProjectile,
		SpawnProjectileEvent{senderFaction, senderID, targetFaction, targetID, damage, startingPosition, endingPosition} };
}