#pragma once

#include "EntityType.h"
#include "glm/glm.hpp"

enum class eFactionController;
class Map;
struct PlayerActivatePlannedBuildingEvent;
class ShaderHandler;
class PlannedBuilding
{
public:
	PlannedBuilding();

	const glm::vec3& getPosition() const;
	int getWorkerID() const;
	eEntityType getEntityType() const;
	bool isActive() const;

	void deactivate();
	void setPosition(const glm::vec3& newPosition, const Map& map);
	void activate(const PlayerActivatePlannedBuildingEvent& gameEvent);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;

private:
	bool m_active;
	int m_workerID;
	glm::vec3 m_position;
	eEntityType m_entityType;
};