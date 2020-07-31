#pragma once

#include "Unit.h"

enum class eHarvesterState
{
	InUseByBaseState = 0,
	MovingToMinerals,
	ReturningMineralsToHQ
};

class Headquarters;
class Harvester : public Unit
{
public:
	Harvester(const glm::vec3& startingPosition, const Model& model, Map& map);
	Harvester(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Model& model, Map& map);
	
	void update(float deltaTime, const ModelManager& modelManager, const Headquarters& HQ, const Map& map);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const Entity& mineral);

private:
	eHarvesterState m_currentHarvesterState;
};