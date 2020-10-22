#pragma once

#include "Entity.h"
#include "NonMovable.h"

class SupplyDepot : public Entity, private NonMovable
{
public:
	SupplyDepot(const glm::vec3& startingPosition);
	~SupplyDepot();
};