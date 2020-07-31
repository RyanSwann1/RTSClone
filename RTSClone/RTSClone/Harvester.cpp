#include "Harvester.h"

//Unit(const glm::vec3& startingPosition, const Model& model, Map& map);
Harvester::Harvester(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Unit(startingPosition, model, map),
	m_currentHarvesterState(eHarvesterState::BaseStateInUse)
{}