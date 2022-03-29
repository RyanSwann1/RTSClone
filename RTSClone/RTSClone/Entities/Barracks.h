#pragma once

#include "EntitySpawnerBuilding.h"

class Barracks : public EntitySpawnerBuilding
{
public:
	Barracks(const glm::vec3& position, Faction& owningFaction);
	Barracks(Barracks&&) noexcept = default;
	Barracks& operator=(Barracks&&) noexcept = default;
};
