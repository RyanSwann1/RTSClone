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

const auto AIPriorityActionCompare = [](const auto& a, const auto& b) -> bool { return b.weight > a.weight; };
using AIPriorityActionQueue = std::priority_queue<AIPriorityAction, std::vector<AIPriorityAction>, decltype(AIPriorityActionCompare)>;

enum class eEntityType;
bool convertActionTypeToEntityType(eAIActionType actionType, eEntityType& entityType);
eAIActionType convertEntityToActionType(eEntityType entityType);