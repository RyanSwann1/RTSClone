#include "GameEvents.h"
#include "Globals.h"

GameEvent_1::GameEvent_1(eEntityType entityType, int targetID)
	: entityType(entityType),
	targetID(targetID)
{}

GameEvent_2::GameEvent_2(eFactionController senderFaction)
	: factionController(senderFaction)
{}

GameEvent_3::GameEvent_3(eFactionController senderFaction, int senderID, eEntityType entityType)
	: factionController(senderFaction),
	entityID(senderID),
	entityType(entityType)
{}

GameEvent_4::GameEvent_4(eFactionController senderFaction, int senderID, eEntityType senderEntityType, 
	eFactionController targetFaction, int targetID, int damage)
	: senderFaction(senderFaction),
	senderID(senderID),
	senderEntityType(senderEntityType),
	targetFaction(targetFaction),
	targetID(targetID),
	damage(damage)
{}

GameEvent_5::GameEvent_5(eFactionController senderFaction, int senderID, eEntityType senderEntityType, 
	eFactionController targetFaction, int targetID, eEntityType targetEntityType, 
	int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition)
	: senderFaction(senderFaction),
	senderID(senderID),
	senderEntityType(senderEntityType),
	targetFaction(targetFaction),
	targetID(targetID),
	targetEntityType(targetEntityType),
	damage(damage),
	spawnPosition(startingPosition),
	destination(endingPosition)
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

GameEvent GameEvent::createAttachFactionToBase(eFactionController factionController, const glm::vec3& position)
{
	return { eGameEventType::AttachFactionToBase, AttachFactionToBaseEvent{factionController, position} };
}

GameEvent GameEvent::createDetachFactionFromBase(eFactionController factionController, const glm::vec3& position)
{
	return { eGameEventType::DetachFactionFromBase, AttachFactionToBaseEvent{factionController, position} };
}

GameEvent GameEvent::create_entity_idle(EntityIdleEvent event)
{
	return { eGameEventType::EntityIdle, event }; 
};

GameEvent GameEvent::create_add_faction_resources(int quantity, eFactionController faction)
{
	return { eGameEventType::AddFactionResources, AddFactionResourcesEvent{quantity, faction} };
}

GameEvent GameEvent::createEliminateFaction(eFactionController factionController)
{
	return { eGameEventType::EliminateFaction, EliminateFactionEvent{factionController} };
}

GameEvent GameEvent::createIncreaseFactionShield(eFactionController factionController)
{
	return { eGameEventType::IncreaseFactionShield, IncreaseFactionShieldEvent{factionController} };	
}

GameEvent GameEvent::createRepairEntity(eFactionController senderFaction, int senderID, eEntityType entityType)
{
	return { eGameEventType::RepairEntity, RepairEntityEvent{senderFaction, senderID, entityType} };
}

GameEvent GameEvent::createSetTargetEntityGUI(eFactionController senderFaction, int senderID, eEntityType entityType)
{
	return { eGameEventType::SetTargetEntityGUI,SetTargetEntityGUIEvent{senderFaction, senderID, entityType} };
}

GameEvent GameEvent::createForceSelfDestructEntity(eFactionController senderFaction, int senderID, eEntityType entityType)
{
	return { eGameEventType::ForceSelfDestructEntity, ForceSelfDestructEntityEvent(senderFaction, senderID, entityType) };
}

GameEvent GameEvent::createTakeDamage(eFactionController senderFaction, int senderID, eEntityType senderEntityType, 
	eFactionController targetFaction, int targetID, int damage)
{
	return { eGameEventType::TakeDamage, TakeDamageEvent{senderFaction, senderID, senderEntityType, targetFaction, targetID, damage} };
}

GameEvent GameEvent::createSpawnProjectile(eFactionController senderFaction, int senderID, eEntityType senderEntityType,
	eFactionController targetFaction, int targetID, eEntityType targetEntityType, int damage, const glm::vec3& startingPosition, const glm::vec3& endingPosition)
{
	return { eGameEventType::SpawnProjectile,
		SpawnProjectileEvent{senderFaction, senderID, senderEntityType, targetFaction, targetID, targetEntityType, damage, startingPosition, endingPosition} };
}