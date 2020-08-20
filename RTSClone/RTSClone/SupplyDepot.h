#pragma once

#include "Entity.h"

class SupplyDepot : public Entity, private NonMovable
{
public:
	SupplyDepot(int ID, const glm::vec3& startingPosition);
	~SupplyDepot();
};