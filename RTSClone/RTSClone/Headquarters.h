#pragma once

#include "Entity.h"
#include <SFML/Graphics.hpp>

struct Camera;
class Headquarters : public Entity
{
public:
	Headquarters(const glm::vec3& startingPosition, const Model& model);

	void handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window);
	void render(ShaderHandler& shaderHandler, const Model& renderModel, const Model& waypointModel) const;

private:
	glm::vec3 m_waypointPosition;
};