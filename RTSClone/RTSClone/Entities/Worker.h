#pragma once

#include "Entity.h"
#include "Movement.h"
#include "Core/Timer.h"
#include "Model/AdjacentPositions.h"
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
	Spawning = 0,
	Idle,
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

struct WorkerScheduledBuilding
{
	WorkerScheduledBuilding(const glm::vec3& position, eEntityType entityType, const int owner_id);

	int owner_id{ UniqueID::INVALID_ID };
	Position position;
	eEntityType entityType;
	std::reference_wrapper<const Model> model;
};

struct EntityToSpawnFromBuilding;
struct Base;
class Headquarters;
class Faction;
class Mineral;
class Worker : public Entity
{
public:
	Worker(Faction& owningFaction, const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map);
	Worker(Worker&&) noexcept = default;
	Worker& operator=(Worker&&) noexcept = default;
	
	const Mineral* getMineralToHarvest() const;
	const std::deque<WorkerScheduledBuilding>& get_scheduled_buildings() const;
	const std::vector<glm::vec3>& getMovementPath() const;
	eWorkerState getCurrentState() const;
	bool is_colliding_with_scheduled_buildings(const AABB& aabb) const;
	bool isHoldingResources() const;
	bool isRepairing() const;
	bool isInBuildQueue(eEntityType entityType) const;
	bool is_group_selectable() const override;
	int extractResources();	

	void clear_destinations();
	bool repairEntity(const Entity& entity, const Map& map) override;
	bool build(const Faction& owningFaction, const glm::vec3& buildPosition, const Map& map, eEntityType entityType,
		bool clearBuildQueue = false);
	bool Harvest(const Mineral& mineral, const Map& map) override;
	void ReturnMineralsToHeadquarters(const Headquarters& headquarters, const Map& map) override;
	bool MoveTo(const glm::vec3& position, const Map& map, const bool add_to_destinations) override;
	void delayed_update(const Map& map, FactionHandler& factionHandler);
	void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	void revalidate_movement_path(const Map& map);

	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;
	void render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const override;
	void renderBuildingCommands(ShaderHandler& shaderHandler) const;
#ifdef RENDER_PATHING
	void render_path(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

private:
	Faction* m_owningFaction							= nullptr;
	Movement m_movement									= {};
	eWorkerState m_currentState							= eWorkerState::Spawning;
	std::deque<WorkerScheduledBuilding> m_buildQueue	= {};
	std::optional<int> m_repairTargetEntity				= {};
	std::optional<int> m_resources						= {};
	Timer m_taskTimer									= {};
	const Mineral* m_mineralToHarvest					= nullptr;

	void switchTo(eWorkerState newState);
	bool move_to(const glm::vec3& destination, const Map& map, const AABB& ignoreAABB, eWorkerState state);
	bool move_to(const glm::vec3& destination, const Map& map, eWorkerState state);
	Entity* CreateBuilding(const WorkerScheduledBuilding& scheduled_building);
};