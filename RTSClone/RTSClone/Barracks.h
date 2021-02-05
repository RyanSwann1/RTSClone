#pragma once

#include "EntitySpawnerBuilding.h"

class Barracks : public EntitySpawnerBuilding
{
public:
	Barracks(const glm::vec3& startingPosition, Faction& owningFaction);

	void update(float deltaTime, const Map& map, FactionHandler& factionHandler);
	bool addUnitToSpawnQueue();
	void renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const;

private:
	const Entity* spawnEntity(const Map& map, FactionHandler& factionHandler) const override;
};