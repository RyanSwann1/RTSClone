#pragma once

#include "Entities/EntityType.h"
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

class Map;
class Model;
class ShaderHandler;
class BaseHandler;
struct Camera;
struct PlayerActivatePlannedBuildingEvent;
class FactionPlayerPlannedBuilding
{
public:
	FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position);

	bool isOnValidPosition(const Map& map) const;
	const glm::vec3& getPosition() const;
	int getBuilderID() const;
	eEntityType getEntityType() const;

	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map);
	void render(ShaderHandler& shaderHandler, const Map& map) const;

private:
	std::reference_wrapper<const Model> m_model;
	int m_builderID;
	eEntityType m_entityType;
	glm::vec3 m_position;
};