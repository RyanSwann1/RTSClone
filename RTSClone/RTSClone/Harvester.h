#pragma once

#include "Unit.h"

enum class eHarvesterState
{
	BaseStateInUse = 0 
};

class Harvester : public Unit
{
public:
	Harvester(const glm::vec3& startingPosition, const Model& model, Map& map);
	
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const Entity& mineral);

private:
	eHarvesterState m_currentHarvesterState;
};