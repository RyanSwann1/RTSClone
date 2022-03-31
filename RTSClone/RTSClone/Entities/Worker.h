#pragma once

#include "Entity.h"
#include "Movement.h"
#include "Timer.h"
#include "AdjacentPositions.h"
#include "TargetEntity.h"
#include <queue>
#include <vector>
#include <deque>
#include <functional>
#include <optional>

//https://stackoverflow.com/questions/50182913/what-are-the-principles-involved-for-an-hierarchical-state-machine-and-how-to-i - HSM
//https://gameprogrammingpatterns.com/state.html

enum class eWorkerState
{
	Idle = 0,
	Moving,
	MovingToMinerals,
	ReturningMineralsToHeadquarters,
	Harvesting,
	Building,
	MovingToRepairPosition,
	Repairing,
	Max = Repairing
};

struct WorkerScheduledBuilding
{
	WorkerScheduledBuilding(const glm::vec3& position, eEntityType entityType);

	glm::vec3 position;
	eEntityType entityType;
	std::reference_wrapper<const Model> model;
};

struct Base;
class Faction;
class Mineral;
class Worker : public Entity
{
public:
	Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, const Map& map);
	Worker(Faction& owningFaction, const glm::vec3& startingPosition, const glm::vec3& startingRotation, 
		const glm::vec3& destination, const Map& map);
	Worker(Worker&&) noexcept = default;
	Worker& operator=(Worker&&) noexcept = default;
	
	const Mineral* getMineralToHarvest() const;
	const std::deque<WorkerScheduledBuilding>& get_scheduled_buildings() const;
	const std::vector<glm::vec3>& getMovementPath() const;
	eWorkerState getCurrentState() const;
	bool isHoldingResources() const;
	bool isRepairing() const;
	bool isInBuildQueue(eEntityType entityType) const;
	int extractResources();	

	void add_destination(const glm::vec3& position, const Map& map);
	void clear_destinations();
	void repairEntity(const Entity& entity, const Map& map);
	bool add_to_build_queue(const Faction& owningFaction, const glm::vec3& buildPosition, const Map& map, eEntityType entityType,
		bool clearBuildQueue = false);
	void update(float deltaTime, const Map& map, const FactionHandler& factionHandler);
	void delayed_update(const Map& map, const FactionHandler& factionHandler);
	void moveTo(const Mineral& mineral, const Map& map);
	void moveTo(const Entity& target, const Map& map, eWorkerState state);
	void moveTo(const glm::vec3& destination, const Map& map);
	void revalidate_movement_path(const Map& map);

	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;
	void renderBuildingCommands(ShaderHandler& shaderHandler) const;
#ifdef RENDER_PATHING
	void render_path(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Faction* m_owningFaction								= nullptr;
	Movement m_movement										= {};
	eWorkerState m_currentState								= eWorkerState::Idle;
	std::deque<WorkerScheduledBuilding> m_buildQueue		= {};
	std::optional<int> m_repairTargetEntity					= {};
	std::optional<int> m_resources							= {};
	Timer m_taskTimer										= {};
	const Mineral* m_mineralToHarvest						= nullptr;

	void switchTo(eWorkerState newState, const Map& map, const Mineral* mineralToHarvest = nullptr);
	void moveTo(const glm::vec3& destination, const Map& map, const AABB& ignoreAABB, eWorkerState state = eWorkerState::Moving);
};