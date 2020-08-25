#pragma once

#include "Unit.h"
#include "Timer.h"

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

class Faction;
class Mineral;
class UnitSpawnerBuilding;
class Worker : public Unit
{
public:
	Worker(const glm::vec3& startingPosition);
	Worker(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Map& map);
	
	bool isHoldingResources() const;
	int extractResources();	

	void build(const std::function<const Entity*(Worker&)>& buildingCommand, const glm::vec3& destination, const Map& map);
	void update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, Faction& owningFaction, const Faction& opposingFaction,
		const std::list<Unit>& units);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, eUnitState state = eUnitState::Moving);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Mineral>& minerals);
	void render(ShaderHandler& shaderHandler) const;

private:
	std::function<const Entity*(Worker&)> m_buildingCommand;
	int m_currentResourceAmount;
	Timer m_harvestTimer;
	const Mineral* m_mineralToHarvest;
};