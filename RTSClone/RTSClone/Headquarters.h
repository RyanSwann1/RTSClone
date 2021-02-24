#pragma once

#include "EntitySpawnerBuilding.h"

class Headquarters : public EntitySpawnerBuilding
{
public:
	Headquarters(const glm::vec3& startingPosition, Faction& owningFaction);
	Headquarters(Headquarters&&) = default;
	Headquarters& operator=(Headquarters&&) = default;
	~Headquarters();

	void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	bool addWorkerToSpawnQueue();
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

private:
	const Entity* spawnEntity(const Map& map, FactionHandler& factionHandler) const override;
};