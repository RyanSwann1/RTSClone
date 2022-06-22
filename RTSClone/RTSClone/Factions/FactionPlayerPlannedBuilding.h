#pragma once

#include "Entities/EntityType.h"
#include "Graphics/Model.h"
#include "Entities/Position.h"
#include "Core/AABB.h"
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

class Map;
struct Model;
class ShaderHandler;
class BaseHandler;
struct Camera;
struct PlayerActivatePlannedBuildingEvent;
class FactionPlayer;
class FactionPlayerPlannedBuilding
{
public:
	FactionPlayerPlannedBuilding(const PlayerActivatePlannedBuildingEvent& gameEvent, const glm::vec3& position, 
		const FactionPlayer* owning_faction);

	bool IsBuildingCreatable(const Map& map) const;
	const glm::vec3& getPosition() const;
	int getBuilderID() const;
	eEntityType getEntityType() const;

	void handleInput(const sf::Event& event, const Camera& camera, const sf::Window& window, const Map& map);
	void render(ShaderHandler& shaderHandler, const Map& map) const;

private:
	const FactionPlayer* m_owning_faction{};
	std::reference_wrapper<const Model> m_model;
	int m_builderID;
	eEntityType m_entityType;
	Position m_position;
	AABB m_aabb{};

	bool isOnValidPosition(const Map& map) const;
};