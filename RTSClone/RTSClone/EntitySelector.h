#pragma once

#include "AABB.h"
#include <SFML/Graphics.hpp>

class ShaderHandler;
struct Camera;
class EntitySelector 
{
public:
	EntitySelector();
	EntitySelector(const EntitySelector&) = delete;
	EntitySelector& operator=(const EntitySelector&) = delete;
	EntitySelector(EntitySelector&&) = delete;
	EntitySelector& operator=(EntitySelector&&) = delete;
	~EntitySelector();

	const AABB& getAABB() const;
	bool isActive() const;

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void update(const Camera& camera, const sf::Window& window);
	void reset();
	void render(const sf::Window& window, ShaderHandler& shaderHandler) const;

private:
	AABB m_AABB;
	bool m_enabled;
	glm::vec2 m_startingMousePosition;
	glm::vec3 m_worldStartingPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;

	bool isMinimumSize() const;
};