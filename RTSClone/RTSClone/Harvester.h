#pragma once

#include "Unit.h"
#include "Timer.h"

enum class eHarvesterState
{
	InUseByBaseState = 0,
	MovingToMinerals,
	ReturningMineralsToHQ,
	Harvesting
};

class Headquarters;
class Harvester : public Unit
{
public:
	Harvester(const glm::vec3& startingPosition, const Model& model, Map& map);
	Harvester(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Model& model, 
		Map& map);
	
	void update(float deltaTime, const ModelManager& modelManager, const Headquarters& HQ, const Map& map, 
		const std::vector<Unit>& units, const std::vector<Harvester>& harvesters);

	void moveTo(const glm::vec3& destinationPosition, const Map& map, 
		const std::vector<Entity>& minerals, const std::vector<Unit>& units);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Unit>& units);

private:
	eHarvesterState m_currentHarvesterState;
	Timer m_harvestTimer;
	const Entity* m_mineralToHarvest;
};