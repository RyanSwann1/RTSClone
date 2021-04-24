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

struct AIOccupiedBase;
struct AIAction
{
	AIAction(eAIActionType actionType, AIOccupiedBase& base);

	eAIActionType actionType;
	std::reference_wrapper<AIOccupiedBase> base;
};

struct AIPriorityAction
{
	AIPriorityAction(int weight, AIAction action);

	int weight;
	AIAction action;
};

const auto AIPriorityActionCompare = [](const auto& a, const auto& b) -> bool { return b.weight > a.weight; };
using AIPriorityActionQueue = std::priority_queue<AIPriorityAction, std::vector<AIPriorityAction>, decltype(AIPriorityActionCompare)>;

enum class eEntityType;
eEntityType convertActionTypeToEntityType(eAIActionType actionType);
eAIActionType convertEntityToActionType(eEntityType entityType);