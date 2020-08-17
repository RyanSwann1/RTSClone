#pragma once

#include "Entity.h"

class Map;
class SupplyDepot : public Entity
{
public:
	SupplyDepot(const glm::vec3& startingPosition, Map& map);
};