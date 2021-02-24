#pragma once

#include "Entity.h"

class SupplyDepot : public Entity
{
public:
	SupplyDepot(const glm::vec3& startingPosition, const Faction& owningFaction);
	SupplyDepot(SupplyDepot&&) = default;
	SupplyDepot& operator=(SupplyDepot&&) = default;
	~SupplyDepot();

	void update(float deltaTime);
};