#include "Laboratory.h"
#include "ModelManager.h"
#include "Globals.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Faction.h"

Laboratory::Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity (ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME), startingPosition, eEntityType::Laboratory,
		Globals::LABORATORY_STARTING_HEALTH, owningFaction.getCurrentShieldAmount())
{
	GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
}

Laboratory::~Laboratory()
{
	GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB });
}
