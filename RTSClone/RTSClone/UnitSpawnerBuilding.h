#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>


class UnitSpawnerBuilding : public Entity, private NonMovable
{
public:
	~UnitSpawnerBuilding();

	bool isWaypointActive() const;
	const glm::vec3& getWaypointPosition() const;
	glm::vec3 getUnitSpawnPosition() const;

	void setWaypointPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;

protected:
	UnitSpawnerBuilding(const glm::vec3& startingPosition, eEntityType entityType);

private:
	glm::vec3 m_waypointPosition;
};

class Barracks : public UnitSpawnerBuilding
{
public:
	Barracks(const glm::vec3& startingPosition);

private:
};

class HQ : public UnitSpawnerBuilding
{
public:
	HQ(const glm::vec3& startingPosition);

private:

};
