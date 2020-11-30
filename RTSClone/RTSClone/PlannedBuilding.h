#pragma once

#include "EntityType.h"
#include "glm/glm.hpp"

enum class eFactionController;
class Map;
struct GameEvent_1;
class ShaderHandler;
class PlannedBuilding
{
public:
	PlannedBuilding();
	PlannedBuilding(int workerID, const glm::vec3& position, eEntityType entityType);

	const glm::vec3& getPosition() const;
	int getWorkerID() const;
	eEntityType getEntityType() const;
	bool isActive() const;

	void setActive(bool active);
	void setPosition(const glm::vec3& newPosition, const Map& map);
	void set(const GameEvent_1& gameEvent);
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController) const;

private:
	bool m_active;
	int m_workerID;
	glm::vec3 m_position;
	eEntityType m_entityType;
};