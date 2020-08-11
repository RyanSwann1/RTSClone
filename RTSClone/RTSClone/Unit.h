#pragma once

#include "Entity.h"
#include "AdjacentPositions.h"
#include <functional>
#include <vector>
#ifdef RENDER_PATHING
#include "Mesh.h"
#endif // RENDER_PATHING

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
	
	void moveTo(const glm::vec3& destinationPosition, const GetAllAdjacentPositions& getAdjacentPositions);
	void update(float deltaTime, const ModelManager& modelManager);

#ifdef RENDER_PATHING
	void renderPathMesh(ShaderHandler& shaderHandler);
#endif // RENDER_PATHING

protected:
	eUnitState m_currentState;
	glm::vec3 m_front;
	std::vector<glm::vec3> m_pathToPosition;

private:
#ifdef RENDER_PATHING
	Mesh m_renderPathMesh;
#endif // RENDER_PATHING
};