#pragma once

#include "Unit.h"
#include "Timer.h"

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

class Faction;
class Mineral;
class Headquarters;
class Worker : public Unit
{
public:
	Worker(const glm::vec3& startingPosition, const Model& model, Map& map);
	Worker(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Model& model, Map& map);
	
	int extractResources();

	void update(float deltaTime, const ModelManager& modelManager, const Headquarters& HQ, const Map& map, Faction& owningFaction);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, eUnitState state = eUnitState::Moving);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals);

private:
	int m_currentResourceAmount;
	Timer m_harvestTimer;
	const Mineral* m_mineralToHarvest;
};