#pragma once

#include "Faction.h"
#include <queue>

enum class eActionType
{
	BuildSupplyDepot,
	BuildBarracks
};

struct AIAction
{
	AIAction(eActionType actionType);
	AIAction(eActionType actionType, const glm::vec3& position);

	eActionType actionType;
	glm::vec3 position;
};

class FactionAI : public Faction
{
public:
	FactionAI(eFactionName factionName, const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition, const Faction& opposingFaction);

	void update(float deltaTime, const Map& map, const Faction& opposingFaction);

private:
	const Faction& m_opposingFaction;
	std::queue<eEntityType> m_spawnQueue;
	std::queue<AIAction> m_actionQueue;
	Timer m_delayTimer;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	const Mineral& getRandomMineral() const;
	Worker* getAvailableWorker(const glm::vec3& position);
};