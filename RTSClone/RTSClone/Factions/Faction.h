#pragma once

#include "Core/PathFinding.h"
#include "Core/Mineral.h"
#include "Core/FactionController.h"
#include "Events/GameMessages.h"
#include "Core/Map.h"
#include "FactionEntities.h"
#include <vector>
#include <functional>
#include <optional>

struct Camera;
struct GameEvent;
class FactionHandler;
class ShaderHandler;
class Map;
class Faction
{
public:
	Faction(const Faction&) = delete;
	Faction& operator=(const Faction&) = delete;
	Faction(Faction&&) noexcept = delete;
	Faction& operator=(Faction&&) noexcept = delete;
	virtual ~Faction() = default;

	bool is_laboratory_built() const;
	bool isMineralInUse(const Mineral& mineral) const;
	bool isExceedPopulationLimit(int populationAmount) const;
	bool isExceedPopulationLimit(eEntityType entityType) const;
	bool isAffordable(eEntityType entityType) const;
	bool isAffordable(int resourceAmount) const;
	bool isCollidingWithWorkerBuildQueue(const AABB& AABB) const;
	bool isBuildingInAllWorkersQueue(eEntityType entityType) const;
	bool IsEntityCreatable(const eEntityType type) const;
	int getCurrentShieldAmount() const;
	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	int get_headquarters_count() const;
	const Headquarters* getMainHeadquarters() const;
	const Headquarters* getClosestHeadquarters(const glm::vec3& position) const;
	eFactionController getController() const;
	const std::vector<Headquarters>& GetHeadquarters() const;
	const std::vector<ConstSafePTR<Entity>>& getEntities() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& aabb, int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;
	const Headquarters* get_closest_headquarters(const glm::vec3& position) const;
	const Entity* get_entity(const int id) const;

	Barracks* CreateBarracks(const WorkerScheduledBuilding& scheduled_building);
	Turret* CreateTurret(const WorkerScheduledBuilding& scheduled_building);
	Headquarters* CreateHeadquarters(const WorkerScheduledBuilding& scheduled_building);
	Laboratory* CreateLaboratory(const WorkerScheduledBuilding& scheduled_building);
	SupplyDepot* CreateSupplyDepot(const WorkerScheduledBuilding& scheduled_building);
	Entity* createUnit(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map);
	Entity* createWorker(const EntityToSpawnFromBuilding& entity_to_spawn, const Map& map);
	virtual bool increaseShield(const Laboratory& laboratory);
	virtual void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler);
	virtual void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	void delayed_update(const Map& map, FactionHandler& factionHandler);
	void render(ShaderHandler& shaderHandler) const;
	void renderPlannedBuildings(ShaderHandler& shaderHandler) const;
	void renderEntityStatusBars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

#ifdef RENDER_PATHING
	void renderPathing(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING
		
#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

protected:
	Faction(eFactionController factionController, const glm::vec3& hqStartingPosition, 
		int startingResources, int startingPopulationCap);

	virtual void on_entity_taken_damage(const TakeDamageEvent& gameEvent, Entity& entity, const Map& map, FactionHandler& factionHandler) {}
	virtual void on_entity_removal(const Entity& entity);
	void on_entity_creation(Entity& entity, const std::optional<int> spawner_id);
	Worker* GetWorker(const int id);

	FactionEntities m_entities{};

private:
	const eFactionController m_controller	= eFactionController::None;
	int m_currentResourceAmount				= 0;
	int m_currentPopulationAmount			= 0;
	int m_currentPopulationLimit			= 0;
	int m_currentShieldAmount				= 0;

	void handleWorkerCollisions(const Map& map);
	

	//Presumes entity already found in all entities container
	template <typename T>
	void removeEntity(std::vector<T>& entityContainer, std::vector<ConstSafePTR<Entity>>::iterator entity);

	template <typename T, typename ...EntityConstructParams>
	T* CreateEntity(std::vector<T>& container, const eEntityType type, 
		const std::optional<int> spawner_id, EntityConstructParams&&... construct_params);
};

template <typename T>
void Faction::removeEntity(std::vector<T>& entityContainer, std::vector<ConstSafePTR<Entity>>::iterator entity)
{
	assert(*(*entity) && entity != m_entities.all.cend());

	const auto iter = std::find_if(entityContainer.begin(), entityContainer.end(), [entity](auto& _entity)
	{
		return _entity.getID() == (*entity)->getID();
	});

	assert(iter != entityContainer.end());

	on_entity_removal(**(*entity));
	m_entities.all.erase(entity);
	entityContainer.erase(iter);
}

template <typename T, typename ...EntityConstructParams>
T* Faction::CreateEntity(std::vector<T>& container, const eEntityType type, 
	const std::optional<int> spawner_id, EntityConstructParams&&... construct_params)
{
	if (IsEntityCreatable(type))
	{
		T* created_entity{ &container.emplace_back(std::forward<EntityConstructParams>(construct_params)...) };
		on_entity_creation(*created_entity, spawner_id);
		return created_entity;
	}

	return nullptr;
}