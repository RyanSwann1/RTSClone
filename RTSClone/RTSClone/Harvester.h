#pragma once

#include "Unit.h"

enum class eHarvesterState
{
	BaseStateInUse
};

class Harvester : public Unit
{
public:
	//Entity(const glm::vec3& startingPosition, const Model& model, eEntityType entityType, Map& map);
	Harvester(const glm::vec3& startingPosition, const Model& model, Map& map);
	
private:
	eHarvesterState m_currentHarvesterState;
};