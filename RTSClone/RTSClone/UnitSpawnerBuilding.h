#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

class UnitSpawnerBuilding : public Entity
{
public:
	UnitSpawnerBuilding(int ID, const glm::vec3& startingPosition, eModelName modelName, eEntityType entityType);
	UnitSpawnerBuilding(UnitSpawnerBuilding&&) noexcept;
	UnitSpawnerBuilding& operator=(UnitSpawnerBuilding&&) noexcept;
	~UnitSpawnerBuilding();

	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void setWaypointPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;

private:
	glm::vec3 m_waypointPosition;
};