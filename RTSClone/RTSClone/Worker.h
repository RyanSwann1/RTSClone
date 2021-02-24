#pragma once

#include "Entity.h"
#include "Timer.h"
#include "AdjacentPositions.h"
#include <queue>
#include <vector>
#include <deque>
#include <functional>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

enum class eWorkerState
{
	Idle = 0,
	Moving,
	MovingToMinerals,
	ReturningMineralsToHeadquarters,
	Harvesting,
	MovingToBuildingPosition,
	Building,
	MovingToRepairPosition,
	Repairing,
	Max = Repairing
};

struct BuildingInWorkerQueue
{
	BuildingInWorkerQueue(const glm::vec3& position, eEntityType entityType);

	const glm::vec3 position;
	const eEntityType entityType;
	const std::reference_wrapper<const Model> model;
};

class Faction;
class Mineral;
class Worker : public Entity
{
public:
	Worker(Faction& owningFaction, const Map& map, const glm::vec3& startingPosition, const glm::vec3& startingRotation);
	Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& destination, const Map& map);
	
	const Mineral* getMineralToHarvest() const;
	const std::list<BuildingInWorkerQueue>& getBuildingCommands() const;
	const std::vector<glm::vec3>& getPathToPosition() const;
	eWorkerState getCurrentState() const;
	bool isHoldingResources() const;
	int extractResources();	

	void repairEntity(const Entity& entity, const Map& map);
	bool build(const glm::vec3& buildPosition, const Map& map, eEntityType entityType);
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const Timer& unitStateHandlerTimer);
	void moveTo(const Mineral& mineral, const Map& map);
	void moveTo(const Entity& target, const Map& map, eWorkerState state);
	void moveTo(const glm::vec3& destination, const Map& map, eWorkerState state = eWorkerState::Moving);

	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void renderBuildingCommands(ShaderHandler& shaderHandler) const;

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	std::reference_wrapper<Faction> m_owningFaction;
	eWorkerState m_currentState;
	std::vector<glm::vec3> m_pathToPosition;
	std::list<BuildingInWorkerQueue> m_buildQueue;
	int m_repairTargetEntityID;
	int m_currentResourceAmount;
	Timer m_taskTimer;
	const Mineral* m_mineralToHarvest;
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING

	void switchTo(eWorkerState newState, const Map& map, const Mineral* mineralToHarvest = nullptr);
	void moveTo(const glm::vec3& destination, const Map& map, const AABB& ignoreAABB, eWorkerState state = eWorkerState::Moving);
};