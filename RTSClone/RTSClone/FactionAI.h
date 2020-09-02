#pragma once

#include "Faction.h"
#include <queue>

enum class eAIImmediateAction
{
	Harvest = 0,
	BuildSupplyDepot
};

struct AIAction
{
	AIAction(eEntityType entityTypeToSpawn, eAIImmediateAction immediateAction);
	AIAction(eEntityType entityTypeToSpawn, eAIImmediateAction immediateAction, const glm::vec3& buildPosition);

	eEntityType entityTypeToSpawn;
	eAIImmediateAction immediateAction;
	glm::vec3 buildPosition;
};

class FactionAI : public Faction
{
public:
	FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition, const Faction& opposingFaction);

	void update(float deltaTime, const Map& map, const Faction& opposingFaction);

private:
	const Faction& m_opposingFaction;
	std::queue<AIAction> m_spawnQueue;
	Timer m_delayTimer;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map);
	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	const Mineral& getRandomMineral() const;
};