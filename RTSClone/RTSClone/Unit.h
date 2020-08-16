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
	Harvesting,
	MovingToBuildingPosition,
	Building
};

class Map;
class ShaderHandler;
class Unit : public Entity
{
public:
	Unit(const glm::vec3& startingPosition, const Model& model, Map& map, eEntityType entityType = eEntityType::Unit);
	Unit(const glm::vec3& startingPosition, const glm::vec3& destinationPosition, const Model& model, Map& map, eEntityType entityType = eEntityType::Unit);

	bool isPathEmpty() const;
	const glm::vec3& getDestination() const;
	eUnitState getCurrentState() const;
	
	void moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Unit>& units,
		const GetAllAdjacentPositions& getAdjacentPositions);
	void moveTo(const glm::vec3& destinationPosition, const Map& map);

	void update(float deltaTime);

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