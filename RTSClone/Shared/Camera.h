#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include <SFML/Graphics.hpp>

struct Camera : private NonCopyable, private NonMovable
{
	Camera();

	glm::mat4 getView() const;
	glm::mat4 getProjection(const sf::Window& window) const;
	glm::vec3 getMouseToGroundPosition(const sf::Window& window) const;

#ifdef LEVEL_EDITOR
	void onMouseMove(const sf::Window& window, float deltaTime, glm::ivec2 lastMousePosition);
	void update(float deltaTime, const sf::Window& window, glm::ivec2 lastMousePosition);
	void zoom(float mouseWheelDelta);
	void zoom(const sf::Window& window, glm::ivec2 lastMousePosition);
#endif // LEVEL_EDITOR

#ifdef GAME
	void update(float deltaTime);
#endif // GAME

	const float FOV;
	const float sensitivity;
	const float nearPlaneDistance;
	const float farPlaneDistance;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec2 rotation;
	glm::vec3 position;
	glm::vec3 velocity;

private:
	void moveByArrowKeys(float deltaTime);
	void setFront();
};