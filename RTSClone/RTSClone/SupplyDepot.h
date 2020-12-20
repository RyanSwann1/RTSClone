#pragma once

#include "Entity.h"

class SupplyDepot : public Entity
{
public:
	SupplyDepot(const glm::vec3& startingPosition, const Faction& owningFaction);
	~SupplyDepot();

	void update(float deltaTime);
};