#pragma once

#include "Entity.h"

class SupplyDepot : public Entity
{
public:
	SupplyDepot(const glm::vec3& startingPosition);
	SupplyDepot(SupplyDepot&&) noexcept;
	SupplyDepot& operator=(SupplyDepot&&) noexcept;
	~SupplyDepot();
};