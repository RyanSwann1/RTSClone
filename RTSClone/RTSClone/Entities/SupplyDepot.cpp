#include "SupplyDepot.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Core/Globals.h"
#include "Graphics/ModelManager.h"
#include "Factions/Faction.h"
#include "Core/Level.h"

SupplyDepot::SupplyDepot(const Position& position, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME), position, 
		eEntityType::SupplyDepot, Globals::SUPPLY_DEPOT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount())
{
	broadcast<GameMessages::AddAABBToMap>({ m_AABB });
	Level::add_event(GameEvent::create<RevalidateMovementPathsEvent>({}));
}

SupplyDepot::~SupplyDepot()
{
	if (getID() != UniqueID::INVALID_ID)
	{
		broadcast<GameMessages::RemoveAABBFromMap>({ m_AABB });
	}
}

bool SupplyDepot::is_group_selectable() const
{
	return false;
}

void SupplyDepot::update(float deltaTime)
{
	Entity::update(deltaTime);
}
