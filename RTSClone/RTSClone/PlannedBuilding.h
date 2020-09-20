#pragma once

#include "EntityType.h"
#include "glm/glm.hpp"

class ShaderHandler;
struct PlannedBuilding
{
	PlannedBuilding();
	PlannedBuilding(int workerID, const glm::vec3& spawnPosition, eEntityType entityType);

	void render(ShaderHandler& shaderHandler) const;

	bool active;
	int workerID;
	glm::vec3 spawnPosition;
	eEntityType entityType;
};