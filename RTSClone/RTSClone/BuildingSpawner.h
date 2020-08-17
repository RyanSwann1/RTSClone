#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

struct Camera;
class BuildingSpawner : public Entity
{
public:
	BuildingSpawner(const glm::vec3& startingPosition, Map& map, eModelName modelName);

	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void setWaypointPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, const Model& renderModel, const Model& waypointModel) const;

private:
	glm::vec3 m_waypointPosition;
};