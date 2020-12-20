#pragma once

#include "Unit.h"
#include "Timer.h"
#include "Sprite.h"
#include <queue>

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

struct BuildingCommand
{
	BuildingCommand(const std::function<const Entity*()>& command, const glm::vec3& buildPosition);

	std::function<const Entity*()> command; //Faction::AddBuilding
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
	
	const Timer& getBuildTimer() const;
	bool isHoldingResources() const;
	int extractResources();	

	void setBuildingToRepair(const Entity& building, const Map& map);
	bool build(const std::function<const Entity*()>& buildingCommand, const glm::vec3& buildPosition, const Map& map);
	void update(float deltaTime, const UnitSpawnerBuilding& HQ, const Map& map, FactionHandler& factionHandler,
		const Timer& unitStateHandlerTimer);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const AdjacentPositions& adjacentPositions,
		eUnitState state = eUnitState::Moving, const Mineral* mineralToHarvest = nullptr);

	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

private:
	std::queue<BuildingCommand> m_buildingCommands;
	int m_repairTargetEntityID;
	int m_currentResourceAmount;
	Timer m_harvestTimer;
	Timer m_buildTimer;
	Timer m_repairTimer;
	Sprite m_buildingProgressSprite;
	const Mineral* m_mineralToHarvest;

	void clearBuildingCommands();
};