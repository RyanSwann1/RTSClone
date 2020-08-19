#pragma once

#include "Entity.h"

class SupplyDepot : public Entity
{
public:
	SupplyDepot(int ID, const glm::vec3& startingPosition);
	SupplyDepot(SupplyDepot&&) noexcept;
	SupplyDepot& operator=(SupplyDepot&&) noexcept;
	~SupplyDepot();
};