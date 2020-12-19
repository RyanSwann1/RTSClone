#pragma once

#include "Entity.h"
#include "NonMovable.h"

class Laboratory : public Entity, private NonMovable
{
public:
	Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction);
	~Laboratory();
};