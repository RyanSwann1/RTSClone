#pragma once

#include "Entity.h"
#include <vector>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

//Unit States
//Moving
//Attacking
//Harvesting 
//Idling

enum class eUnitState
{
	Idle = 0,
	Moving,
	MovingToMinerals,
	ReturningMineralsToHQ,
	Harvesting
};

class Map;
class ShaderHandler;
class ModelManager;
class Unit : public Entity
{
public:
	Unit(const glm::vec3& startingPosition, const Model& model, Map& map, eEntityType entityType = eEntityType::Unit);
	Unit(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Model& model, Map& map, eEntityType entityType = eEntityType::Unit);

	eUnitState getCurrentState() const;

	void moveToAmongstGroup(const glm::vec3& destinationPosition, const Map& map, 
		const std::vector<Unit>& units, const std::vector<const Unit*>& selectedUnits);
	void moveToAmongstGroup(const glm::vec3& destinationPosition, const Map& map);
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Unit>& units);
	void moveTo(const glm::vec3& destinationPosition, const Map& map);
	void moveToAStar(const glm::vec3& destinationPosition, const Map& map, const std::vector<Unit>& units);
	void update(float deltaTime, const ModelManager& modelManager);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	eUnitState m_currentState;
	glm::vec3 m_front;
	std::vector<glm::vec3> m_pathToPosition;

#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};