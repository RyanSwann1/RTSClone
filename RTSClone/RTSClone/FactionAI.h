#pragma once

#include "Faction.h"
#include <queue>

class FactionAI : public Faction
{
public:
	FactionAI(const glm::vec3& hqStartingPosition, const glm::vec3& mineralsStartingPosition);

	void update(float deltaTime, const Map& map, const Faction& opposingFaction);

private:
	std::queue<eEntityType> m_unitSpawnQueue;
	Timer m_delayTimer;
};