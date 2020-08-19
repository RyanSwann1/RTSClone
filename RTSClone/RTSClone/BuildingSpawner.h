#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

struct Camera;
class BuildingSpawner : public Entity
{
public:
	BuildingSpawner(const glm::vec3& startingPosition, eModelName modelName);
	BuildingSpawner(BuildingSpawner&&) noexcept;
	BuildingSpawner& operator=(BuildingSpawner&&) noexcept;
	~BuildingSpawner();

	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void setWaypointPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;

private:
	glm::vec3 m_waypointPosition;
};