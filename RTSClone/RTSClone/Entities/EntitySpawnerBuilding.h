#pragma once

#include "Timer.h"
#include "Entity.h"
#include <functional>
#include <queue>
#include <optional>

struct EntitySpawnerDetails
{
	float timeBetweenSpawn		= 0.f;
	float progressBarWidth		= 0.f;
	float progressBarYOffset	= 0.f;
	int maxSpawnCount			= 0;
	int resourceCost			= 0;
	int populationCost			= 0;
};

class Map;
class Faction;
class EntitySpawnerBuilding : public Entity
{
public:
	EntitySpawnerBuilding(const glm::vec3& position, const eEntityType type, const int health,
		const int shield, EntitySpawnerDetails spawnDetails,
		std::function<Entity*(Faction&, const Map&, const EntitySpawnerBuilding&)> spawnCallback);
	EntitySpawnerBuilding(EntitySpawnerBuilding&&) noexcept = default;
	EntitySpawnerBuilding& operator=(EntitySpawnerBuilding&&) noexcept = default;
	~EntitySpawnerBuilding();

	int get_current_spawn_count() const;
	std::optional<glm::vec3> get_waypoint() const;

	void update(const float deltaTime, Faction& owningFaction, const Map& map);
	void render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const override;
	void set_waypoint_position(const Map& map, const glm::vec3& position);
	bool add_entity_to_spawn_queue(const Faction& owningFaction);
	void render_waypoint(ShaderHandler& shaderHandler) const;

protected:
	Timer m_timer								= {};

private:
	EntitySpawnerDetails m_details				= {};
	int m_spawnCount							= 0;
	std::optional<glm::vec3> m_waypoint			= {};
	std::function<Entity*(Faction&, const Map&, const EntitySpawnerBuilding&)> m_spawnCallback = {};
};