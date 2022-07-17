#pragma once

#include <queue>

enum class AIActionnn
{
	BuildSupplyDepot,
	BuildBarracks,
	BuildTurret,
	BuildLaboratory,
	SpawnUnit,
	SpawnWorker,
	IncreaseShield
};

enum class AIBaseAction
{

};

enum class AIEntityAction
{

};

class FactionAI;
class FactionAIActionHandler
{
public:
	FactionAIActionHandler(FactionAI* owning_faction);

private:
	FactionAI* m_owning_faction{ nullptr };
	std::queue<AIActionnn> m_action_queue{};
};