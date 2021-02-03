#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "AABB.h"
#include <SFML/Graphics.hpp>

struct Camera;
class SelectionBox : private NonCopyable, private NonMovable
{
public:
	SelectionBox();
	~SelectionBox();

	const AABB& getAABB() const;
	bool isActive() const;
	bool isMinimumSize() const;

	void setStartingPosition(const sf::Window& window, const glm::vec3& position);
	void update(const Camera& camera, const sf::Window& window);
	void reset();
	void render(const sf::Window& window) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	AABB m_AABB;
	bool m_active;
	glm::vec2 m_screenStartingPosition;
	glm::vec3 m_worldStartingPosition;
	unsigned int m_vaoID;
	unsigned int m_vboID;
};