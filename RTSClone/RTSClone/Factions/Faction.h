#pragma once

#include "Entities/Headquarters.h"
#include "Entities/Barracks.h"
#include "Entities/Worker.h"
#include "Entities/Unit.h"
#include "Entities/Turret.h"
#include "Entities/Laboratory.h"
#include "Core/PathFinding.h"
#include "Entities/SupplyDepot.h"
#include "Core/Mineral.h"
#include "Core/FactionController.h"
#include "Events/GameMessages.h"
#include "Core/Map.h"
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
	int getCurrentShieldAmount() const;
	int getCurrentPopulationAmount() const;
	int getMaximumPopulationAmount() const;
	int getCurrentResourceAmount() const;
	int get_headquarters_count() const;
	const Headquarters* getMainHeadquarters() const;
	const Headquarters* getClosestHeadquarters(const glm::vec3& position) const;
	eFactionController getController() const;
	const std::vector<Entity*>& getEntities() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& aabb, int entityID) const;
	const Entity* getEntity(const glm::vec3& position) const;
	const Headquarters* get_closest_headquarters(const glm::vec3& position) const;
	const Entity* get_entity(const int id) const;

	virtual Barracks* CreateBarracks(const WorkerScheduledBuilding& scheduled_building);
	virtual Turret* CreateTurret(const WorkerScheduledBuilding& scheduled_building);
	virtual Headquarters* CreateHeadquarters(const WorkerScheduledBuilding& scheduled_building);
	virtual Laboratory* CreateLaboratory(const WorkerScheduledBuilding& scheduled_building);
	virtual SupplyDepot* CreateSupplyDepot(const WorkerScheduledBuilding& scheduled_building);
	virtual Entity* createUnit(const Map& map, const EntitySpawnerBuilding& spawner);
	virtual Entity* createWorker(const Map& map, const EntitySpawnerBuilding& spawner);
	virtual bool increaseShield(const Laboratory& laboratory);
	virtual void handleEvent(const GameEvent& gameEvent, const Map& map, FactionHandler& factionHandler, 
		const BaseHandler& baseHandler);
	virtual void update(float deltaTime, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler);
	void delayed_update(const Map& map, FactionHandler& factionHandler);
	virtual void render(ShaderHandler& shaderHandler) const;
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
	virtual void on_entity_idle(Entity& entity, const Map& map, FactionHandler& factionHandler, const BaseHandler& baseHandler) {}
	virtual void on_entity_removal(const Entity& entity);
	Worker* GetWorker(const int id);

	std::vector<Entity*> m_allEntities;
	std::vector<Unit> m_units;
	std::vector<Worker> m_workers;
	std::vector<SupplyDepot> m_supplyDepots;
	std::vector<Barracks> m_barracks;
	std::vector<Turret> m_turrets;
	std::vector<Headquarters> m_headquarters;
	std::vector<Laboratory> m_laboratories;

private:
	const eFactionController m_controller	= eFactionController::None;
	int m_currentResourceAmount				= 0;
	int m_currentPopulationAmount			= 0;
	int m_currentPopulationLimit			= 0;
	int m_currentShieldAmount				= 0;

	void handleWorkerCollisions(const Map& map);
	void on_entity_creation(Entity& entity);
	bool is_entity_creatable(eEntityType type, const size_t current, const size_t max) const;

	//Presumes entity already found in all entities container
	template <typename T>
	void removeEntity(std::vector<T>& entityContainer, std::vector<Entity*>::iterator entity);

	template <typename T>
	Entity* CreateEntityFromBuilding(const Map& map, const EntitySpawnerBuilding& spawner, const eEntityType type, 
		std::vector<T>& entityContainer, const int maxEntityCount);

	template <typename T>
	T* CreateBuilding(std::vector<T>& container, const WorkerScheduledBuilding& scheduled_building, const size_t max_size);
};

template <typename T>
void Faction::removeEntity(std::vector<T>& entityContainer, std::vector<Entity*>::iterator entity)
{
	assert((*entity) && entity != m_allEntities.cend());

	const auto iter = std::find_if(entityContainer.begin(), entityContainer.end(), [entity](auto& _entity)
	{
		return _entity.getID() == (*entity)->getID();
	});

	assert(iter != entityContainer.end());

	on_entity_removal(*(*entity));
	m_allEntities.erase(entity);
	entityContainer.erase(iter);
}

template<typename T>
inline Entity* Faction::CreateEntityFromBuilding(const Map& map, const EntitySpawnerBuilding& spawner, const eEntityType type, 
	std::vector<T>& entityContainer, const int maxEntityCount)
{
	glm::vec3 startingPosition(0.0f);
	if (is_entity_creatable(type, entityContainer.size(), maxEntityCount) 
		&& PathFinding::getInstance().getClosestAvailableEntitySpawnPosition(spawner, map, startingPosition))
	{
		glm::vec3 startingRotation = { 0.0f, Globals::getAngle(startingPosition, spawner.getPosition()), 0.0f };
		Entity* createdEntity = nullptr;
		if (spawner.get_waypoint())
		{
			createdEntity = &entityContainer.emplace_back(*this, startingPosition, startingRotation, *spawner.get_waypoint(), map);
		}
		else
		{
			createdEntity = &entityContainer.emplace_back(*this, startingPosition, startingRotation, map);
		}

		on_entity_creation(*createdEntity);
		return createdEntity;
	}

	return nullptr;
}

template<typename T>
inline T* Faction::CreateBuilding(std::vector<T>& container, const WorkerScheduledBuilding& scheduled_building, const size_t max_size)
{
	if (is_entity_creatable(scheduled_building.entityType, container.size(), max_size))
	{
		container.emplace_back(scheduled_building.position, *this);
		on_entity_creation(container.back());
		return &container.back();
	}

	return nullptr;
}