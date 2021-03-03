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
	broadcastToMessenger<GameMessages::AddBuildingToMap>({ *this });
}

SupplyDepot::~SupplyDepot()
{
	if (m_status.isActive())
	{
		broadcastToMessenger<GameMessages::RemoveBuildingFromMap>({ *this });
	}
}

void SupplyDepot::update(float deltaTime)
{
	Entity::update(deltaTime);
}
