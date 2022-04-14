#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "ModelManager.h"
#include "Factions/Faction.h"
#include "Level.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME), startingPosition, eEntityType::SupplyDepot, 
		Globals::SUPPLY_DEPOT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount())
{
	broadcast<GameMessages::AddAABBToMap>({ m_AABB });
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

SupplyDepot::~SupplyDepot()
{
	if (getID() != INVALID_ENTITY_ID)
	{
		broadcast<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

void SupplyDepot::update(float deltaTime)
{
	Entity::update(deltaTime);
}
