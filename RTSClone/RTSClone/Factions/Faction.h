#pragma once

#include "Headquarters.h"
#include "Barracks.h"
#include "Worker.h"
#include "PathFinding.h"
#include "SupplyDepot.h"
#include "Mineral.h"
#include "FactionController.h"
#include "Turret.h"
#include "Laboratory.h"
#include "GameMessages.h"
#include "Map.h"
#include <vector>
#include <functional>
#include <memory>
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
	Faction(Faction&&) = delete;
	Faction& operator=(Faction&&) = delete;
	virtual ~Faction() {}

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
	int getLaboratoryCount() const;
	const Headquarters* getMainHeadquarters() const;
	const Headquarters* getClosestHeadquarters(const glm::vec3& position) const;
	eFactionController getController() const;
	const std::vector<std::unique_ptr<Entity>>& getEntities() const;
	const Entity* getEntity(const glm::vec3& position, float maxDistance, bool prioritizeUnits = true) const;
	const Entity* getEntity(const AABB& AABB, int entityID, eEntityType entityType) const;
	const Entity* getEntity(int entityID, eEntityType entityType) const;
	const Entity* getEntity(const glm::vec3& position) const;

	virtual Entity* createUnit(const Map& map, const Barracks& barracks, const FactionHandler& factionHandler);
	virtual Entity* createWorker(const Map& map, const Headquarters& headquarters);
	virtual bool increaseShield(const Laboratory& laboratory);
	virtual void handleEvent(const GameEvent& gameEvent, const Map& map, const FactionHandler& factionHandler, 
		const BaseHandler& baseHandler);
	virtual void update(float deltaTime, const Map& map, const FactionHandler& factionHandler, 
		const Timer& unitStateHandlerTimer, const BaseHandler& baseHandler);
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

	virtual void on_entity_taken_damage(const TakeDamageEvent& gameEvent, Entity& entity, const Map& map, const FactionHandler& factionHandler) {}
	virtual void on_entity_idle(Entity& entity, const Map& map, const FactionHandler& factionHandler, const BaseHandler& baseHandler) {}
	virtual void onEntityRemoval(const Entity& entity) {}

	virtual Entity* create_building(const GameMessages::CreateBuilding& message);

	std::vector<std::unique_ptr<Entity>> m_entities;
	std::vector<Unit*> m_units;
	std::vector<Worker*> m_workers;
	std::vector<SupplyDepot*> m_supplyDepots;
	std::vector<Barracks*> m_barracks;
	std::vector<Turret*> m_turrets;
	std::vector<Headquarters*> m_headquarters;
	std::vector<Laboratory*> m_laboratories;

private:
	const eFactionController m_controller	= eFactionController::None;
	int m_currentResourceAmount				= 0;
	int m_currentPopulationAmount			= 0;
	int m_currentPopulationLimit			= 0;
	int m_currentShieldAmount				= 0;
	GameMessengerSubscriber<GameMessages::GetClosestHeadquarters,
		eFactionController, const Headquarters*> m_getClosestHeadquatersSubscriber;
	GameMessengerSubscriber<GameMessages::GetEntity, eFactionController, const Entity*> m_getEntitySubscriber;
	GameMessengerSubscriber<GameMessages::CreateBuilding, eFactionController, const Entity*> m_createBuildingSubscriber;

	void reduceResources(eEntityType addedEntityType);
	void increaseCurrentPopulationAmount(eEntityType entityType);
	void decreaseCurrentPopulationAmount(const Entity& entity);
	void increasePopulationLimit();
	void revalidateExistingUnitPaths(const Map& map);
	void handleWorkerCollisions(const Map& map);

	const Headquarters* get_closest_headquarters(const GameMessages::GetClosestHeadquarters& message) const;
	const Entity* get_entity(const GameMessages::GetEntity& message) const;

	//Presumes entity already found in all entities container
	template <typename T>
	void removeEntity(std::vector<T>& entityContainer, int entityID,
		std::vector<std::unique_ptr<Entity>>::iterator entity);

	template <typename T>
	void removeEntity(std::vector<T>& entityContainer, int entityID);

	template <typename T>
	const Entity* getEntity(const T& entityContainer, int entityID) const;

	template <typename T> 
	const Entity* getEntity(const T& entityContainer, int entityID, const AABB& entityAABB) const;
	
	template <typename T>
	Entity* createEntity(std::vector<T*>& handleContainer, const glm::vec3& position);
};

template <typename T>
void Faction::removeEntity(std::vector<T>& entityContainer, int entityID, std::vector<std::unique_ptr<Entity>>::iterator entity)
{
	assert(entity != m_entities.cend());

	auto iter = std::find_if(entityContainer.begin(), entityContainer.end(), [entityID](const auto& entity)
	{
		return entity->getID() == entityID;
	});

	assert(iter != entityContainer.end());

	onEntityRemoval(*(*entity));
	m_entities.erase(entity);
	entityContainer.erase(iter);
}

template <typename T>
void Faction::removeEntity(std::vector<T>& handleContainer, int entityID)
{
	auto iter = std::find_if(m_entities.begin(), m_entities.end(), [entityID](const auto& entity)
	{
		return entity->getID() == entityID;
	});
	if (iter != m_entities.end())
	{
		auto entity = std::find_if(handleContainer.begin(), handleContainer.end(), [entityID](const auto& entity)
		{
			return entity->getID() == entityID;
		});
		assert(entity != handleContainer.end());

		onEntityRemoval(*(*entity));
		m_entities.erase(iter);
		handleContainer.erase(entity);
	}
}

template <typename T>
const Entity* Faction::getEntity(const T& entityContainer, int entityID) const
{
	auto entity = std::find_if(entityContainer.cbegin(), entityContainer.cend(), [entityID](const auto& entity)
	{
		return entity->getID() == entityID;
	});
	return (entity != entityContainer.cend() ? &*(*entity) : nullptr);
}

template <typename T>
const Entity* Faction::getEntity(const T& entityContainer, int entityID, const AABB& entityAABB) const
{
	auto entity = std::find_if(entityContainer.cbegin(), entityContainer.cend(), [&entityAABB, entityID](const auto& entity)
	{
		return entity->getID() == entityID && entity->getAABB().contains(entityAABB);
	});
	return (entity != entityContainer.cend() ? &*(*entity) : nullptr);
}

template <typename T>
Entity* Faction::createEntity(std::vector<T*>& handleContainer, const glm::vec3& position)
{
	Entity* addedEntity = m_entities.emplace_back(std::make_unique<T>(position, *this)).get();
	handleContainer.push_back(static_cast<T*>(addedEntity));

	return addedEntity;
}