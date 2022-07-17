#pragma once

#include "Faction.h"
#include "FactionAIBase.h"
#include "FactionAIEntityHandler.h"
#include "Core/Graph.h"
#include "Core/Timer.h"
#include "AI/AIOccupiedBases.h"
#include "AI/AIAction.h"
#include "AI/AIConstants.h"
#include "AI/AIUnattachedToBaseWorkers.h"
#include <queue>
#include <vector>
#include <functional>

enum class FactionAIAction
{
	BuildWorker = 0
};

class HarvestLocationManager;
struct EntityIdleEvent;
class FactionAI_NEW final : public Faction
{
public:
	FactionAI_NEW(eFactionController factionController, const glm::vec3& hqStartingPosition,
		int startingResources, int startingPopulationCap, AIConstants::eBehaviour behaviour, 
		const HarvestLocationManager& harvest_location_manager);

	void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler) override;
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler) override;

private:
	const HarvestLocationManager& m_harvest_location_manager;
	FactionAIBaseManager m_occupied_base_manager;
	FactionAIEntityHandler m_entity_handler;
	std::queue<FactionAIAction> m_action_queue{};

	bool RequestWorkerCreation();

	void OnWorkerIdle(Worker& worker, const Map& map);
};