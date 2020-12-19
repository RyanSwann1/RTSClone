#include "SupplyDepot.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Globals.h"
#include "ModelManager.h"
#include "Faction.h"

SupplyDepot::SupplyDepot(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(SUPPLY_DEPOT_MODEL_NAME), startingPosition, eEntityType::SupplyDepot, 
		Globals::SUPPLY_DEPOT_STARTING_HEALTH, owningFaction.getCurrentShieldAmount())
{
	GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
}

SupplyDepot::~SupplyDepot()
{
	GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB });
}