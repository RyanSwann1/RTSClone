#include "AIAction.h"
#include "EntityType.h"
#include <assert.h>

//AIAction
AIAction::AIAction(eAIActionType actionType, AIOccupiedBase& base)
	: actionType(actionType),
	base(base)
{}

//AIPriorityAction
AIPriorityAction::AIPriorityAction(int weight, AIAction action)
	: weight(weight),
	action(action)
{}

eEntityType convertActionTypeToEntityType(eAIActionType actionType)
{
	eEntityType entityType;
	switch (actionType)
	{
	case eAIActionType::BuildSupplyDepot:
		entityType = eEntityType::SupplyDepot;
		break;
	case eAIActionType::BuildBarracks:
		entityType = eEntityType::Barracks;
		break;
	case eAIActionType::BuildTurret:
		entityType = eEntityType::Turret;
		break;
	case eAIActionType::BuildLaboratory:
		entityType = eEntityType::Laboratory;
		break;
	case eAIActionType::SpawnUnit:
		entityType = eEntityType::Unit;
		break;
	case eAIActionType::SpawnWorker:
		entityType = eEntityType::Worker;
		break;
	default:
		assert(false);
	}

	return entityType;
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