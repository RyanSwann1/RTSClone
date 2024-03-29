#pragma once

#include "Core/AABB.h"
#include "Graphics/OpenGLResource.h"
#include <SFML/Graphics.hpp>

class ShaderHandler;
struct Camera;
class EntitySelectorBox 
{
public:
	const AABB& getAABB() const;
	bool isActive() const;

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void update(const Camera& camera, const sf::Window& window);
	void reset();
	void render(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	AABB m_AABB;
	bool m_enabled = false;
	glm::vec2 m_startingMousePosition;
	glm::vec3 m_worldStartingPosition;
	OpenGLResourceVertexArray m_VAO;
	OpenGLResourceBuffer m_VBO = { GL_ARRAY_BUFFER };

	bool isMinimumSize() const;
};