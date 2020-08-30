#pragma once

#include "Unit.h"
#include "Timer.h"
#include <queue>

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

struct BuildingCommand
{
	BuildingCommand(const std::function<const Entity* (Worker&)>& command, const glm::vec3& buildPosition);

	std::function<const Entity*(Worker&)> command;
	glm::vec3 buildPosition;
};

class Faction;
class Mineral;
class UnitSpawnerBuilding;
class Worker : public Unit
{
public:
	Worker(const Faction& owningFaction, const glm::vec3& startingPosition);
	Worker(const Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Map& map);
	~Worker();
	
	bool isHoldingResources() const;
	int extractResources();	

	bool build(const std::function<const Entity*(Worker&)>& buildingCommand, const glm::vec3& buildPosition, const Map& map);
	void update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, const Faction& opposingFaction,
		const std::list<Unit>& units);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const GetAllAdjacentPositions& getAdjacentPositions, 
		eUnitState state = eUnitState::Moving, const Mineral* mineralToHarvest = nullptr);
	void render(ShaderHandler& shaderHandler) const;

private:
	std::queue<BuildingCommand> m_buildingCommands;
	int m_currentResourceAmount;
	Timer m_harvestTimer;
	Timer m_buildTimer;
	const Mineral* m_mineralToHarvest;

	void clearBuildingCommands();
};