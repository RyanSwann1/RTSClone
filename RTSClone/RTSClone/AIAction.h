#pragma once

#include <functional>
#include <queue>
#include <vector>

enum class eAIActionType
{
	BuildSupplyDepot,
	BuildBarracks,
	BuildTurret,
	BuildLaboratory,
	SpawnUnit,
	SpawnWorker,
	IncreaseShield
};

struct AIAction
{
	AIAction(eAIActionType actionType);

	eAIActionType actionType;
};

struct AIPriorityAction : public AIAction
{
	AIPriorityAction(int weight, eAIActionType actionType);

	int weight;
};

struct AIPriorityActionCompare {
	bool operator()(const AIPriorityAction& a, const AIPriorityAction& b) { return b.weight > a.weight; }
};
using AIPriorityActionQueue = std::priority_queue<AIPriorityAction, std::vector<AIPriorityAction>, AIPriorityActionCompare>;

enum class eEntityType;
bool convertActionTypeToEntityType(eAIActionType actionType, eEntityType& entityType);
eAIActionType convertEntityToActionType(eEntityType entityType);