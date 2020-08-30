#pragma once

#include "Faction.h"
#include <queue>

struct AIAction
{
	AIAction(eEntityType entityTypeToSpawn, const glm::vec3& spawnPosition);

	eEntityType entityTypeToSpawn;
	glm::vec3 spawnPosition;
};

class FactionAI : public Faction
{
public:
	FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition);

	void update(float deltaTime, const Map& map, const Faction& opposingFaction);

private:
	std::queue<AIAction> m_spawnQueue;
	Timer m_delayTimer;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map);
};