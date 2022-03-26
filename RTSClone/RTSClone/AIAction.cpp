#include "AIAction.h"
#include "Entities/EntityType.h"
#include <assert.h>

//AIAction
AIAction::AIAction(eAIActionType actionType)
	: actionType(actionType)
{}

//AIPriorityAction
AIPriorityAction::AIPriorityAction(int weight, eAIActionType actionType)
	: AIAction(actionType),
	weight(weight)
{}

bool convertActionTypeToEntityType(eAIActionType actionType, eEntityType& entityType)
{
	switch (actionType)
	{
	case eAIActionType::BuildSupplyDepot:
		entityType = eEntityType::SupplyDepot;
		return true;
	case eAIActionType::BuildBarracks:
		entityType = eEntityType::Barracks;
		return true;
	case eAIActionType::BuildTurret:
		entityType = eEntityType::Turret;
		return true;
	case eAIActionType::BuildLaboratory:
		entityType = eEntityType::Laboratory;
		return true;
	case eAIActionType::SpawnUnit:
		entityType = eEntityType::Unit;
		return true;
	case eAIActionType::SpawnWorker:
		entityType = eEntityType::Worker;
		return true;
	}

	return false;
}

eAIActionType convertEntityToActionType(eEntityType entityType)
{
	eAIActionType actionType;
	switch (entityType)
	{
	case eEntityType::SupplyDepot:
		actionType = eAIActionType::BuildSupplyDepot;
		break;
	case eEntityType::Barracks:
		actionType = eAIActionType::BuildBarracks;
		break;
	case eEntityType::Turret:
		actionType = eAIActionType::BuildTurret;
		break;
	case eEntityType::Laboratory:
		actionType = eAIActionType::BuildLaboratory;
		break;
	default:
		assert(false);
	}

	return actionType;
}