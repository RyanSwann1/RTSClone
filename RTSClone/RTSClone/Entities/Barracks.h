#pragma once

#include "EntitySpawnerBuilding.h"

class Barracks : public EntitySpawnerBuilding
{
public:
	Barracks(const Position& position, Faction& owningFaction);
	Barracks(Barracks&&) noexcept = default;
	Barracks& operator=(Barracks&&) noexcept = default;
};