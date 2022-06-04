#pragma once

#include "Core/Timer.h"
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

struct EntityToSpawnFromBuilding
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	std::optional<glm::vec3> destination{};
	eEntityType type{};
	glm::vec3 building_position{};
};

class Map;
class Faction;
class EntitySpawnerBuilding : public Entity
{
public:
	EntitySpawnerBuilding(const Position& position, const eEntityType type, const int health,
		const int shield, EntitySpawnerDetails spawnDetails);

	EntitySpawnerBuilding(EntitySpawnerBuilding&&) noexcept = default;
	EntitySpawnerBuilding& operator=(EntitySpawnerBuilding&&) noexcept = default;
	~EntitySpawnerBuilding();

	int get_current_spawn_count() const;
	std::optional<glm::vec3> get_waypoint() const;
	bool is_group_selectable() const override;

	void update(const float deltaTime, Faction& owningFaction, const Map& map);
	void render_status_bars(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const override;
	bool set_waypoint_position(const glm::vec3& position, const Map& map) override;
	bool AddEntityToSpawnQueue(const Faction& owningFaction) override;
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const override;

protected:
	Timer m_timer								= {};

private:
	EntitySpawnerDetails m_details				= {};
	int m_spawnCount							= 0;
	std::optional<glm::vec3> m_waypoint			= {};

	virtual const Entity* CreateEntity(Faction& owning_faction, const Map& map) = 0;
};