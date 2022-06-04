#pragma once

#include "EntitySpawnerBuilding.h"

class Headquarters : public EntitySpawnerBuilding
{
public:
	Headquarters(const Position& position, Faction& owningFaction);
	Headquarters(Headquarters&&) = default;
	Headquarters& operator=(Headquarters&&) = default;
	~Headquarters();

private:
	const Faction* m_owningFaction = nullptr;
};