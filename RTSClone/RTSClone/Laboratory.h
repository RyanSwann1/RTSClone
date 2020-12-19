#pragma once

#include "Entity.h"

class Laboratory : public Entity
{
public:
	Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction);
	~Laboratory();
};