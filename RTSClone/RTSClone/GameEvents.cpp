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
	return { eGameEventType::PlayerSpawnUnit, PlayerSpawnUnitEvent{entityType, targetID} };
}

GameEvent GameEvent::createPlayerActivatePlannedBuilding(eEntityType entityType, int targetID)
{
	return { eGameEventType::PlayerActivatePlannedBuilding, PlayerActivatePlannedBuildingEvent{entityType, targetID} };
}

GameEvent GameEvent::createEliminateFaction(eFactionController factionController)
{
	return { eGameEventType::EliminateFaction, EliminateFactionEvent{factionController} };
}

GameEvent GameEvent::createIncreaseFactionShield(eFactionController factionController)
{
	return { eGameEventType::IncreaseFactionShield, IncreaseFactionShieldEvent{factionController} };	
}

GameEvent GameEvent::createAddResources(eFactionController senderFaction, int senderID)
{
	return { eGameEventType::AddResources, AddResourcesEvent{senderFaction, senderID} };
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