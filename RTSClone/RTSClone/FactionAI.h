#pragma once

#include "Faction.h"
#include "Graph.h"
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

class FactionHandler;
class FactionAI : public Faction
{
public:
	FactionAI(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		const std::array<glm::vec3, Globals::MAX_MINERALS_PER_FACTION>& mineralPositions);

	void update(float deltaTime, const Map& map, FactionHandler& factionHandler) override;

private:
	std::queue<eEntityType> m_spawnQueue;
	std::queue<AIAction> m_actionQueue;
	Timer m_delayTimer;
	Graph m_graph;
	std::queue<glm::ivec2> m_frontier;

	bool instructWorkerToBuild(eEntityType entityType, const glm::vec3& position, const Map& map, Worker& worker);
	const Mineral& getRandomMineral() const;
	Worker* getAvailableWorker(const glm::vec3& position);

	bool isBuildingSpawnAvailable(const glm::vec3& startingPosition, eEntityType entityTypeToBuild, const Map& map, 
		glm::vec3& buildPosition);
};