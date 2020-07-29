#include "Headquarters.h"
#include "Camera.h"
#include "Model.h"

Headquarters::Headquarters(const glm::vec3& startingPosition, const Model& model)
	: Entity(startingPosition, model),
	m_waypointPosition(startingPosition)
{}

void Headquarters::handleInput(const sf::Event& currentSFMLEvent, const Camera& camera, const sf::Window& window)
{
	if (m_selected && currentSFMLEvent.type == sf::Event::MouseButtonPressed &&
		currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
	{
		glm::vec3 position = camera.getMouseToGroundPosition(window);
		if (m_AABB.contains(position))
		{
			m_waypointPosition = m_position;
		}
		else
		{
			m_waypointPosition = position;
		}
	}
}

void Headquarters::render(ShaderHandler & shaderHandler, const Model & renderModel, const Model & waypointModel) const
{
	if (m_selected && m_waypointPosition != m_position)
	{
		waypointModel.render(shaderHandler, m_waypointPosition);
	}
	
	renderModel.render(shaderHandler, *this);
}