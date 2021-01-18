#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include <SFML/Graphics.hpp>

struct Camera : private NonCopyable, private NonMovable
{
	Camera();

	glm::mat4 getView() const;
	glm::mat4 getProjection(glm::ivec2 windowSize) const;

#ifdef LEVEL_EDITOR
	//Return false if ground not found
	bool getRayToGroundIntersection(const sf::Window& window, glm::uvec2 windowSize, glm::vec3& intersection) const;
	void onMouseMove(const sf::Window& window, float deltaTime);
	void update(float deltaTime, const sf::Window& window, glm::ivec2 lastMousePosition);
	void zoom(int mouseWheelDelta);
	void zoom(const sf::Window& window, glm::ivec2 lastMousePosition);
#endif // LEVEL_EDITOR

#ifdef GAME
	glm::vec3 getInfiniteForwardRay(const sf::Window& window) const;
	glm::vec3 getRayToGroundPlaneIntersection(const sf::Window& window) const;
	void update(float deltaTime, const sf::Window& window, glm::uvec2 windowSize);
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
#ifdef GAME
	void moveByMouse(float deltaTime, const sf::Window& window, glm::uvec2 windowSize);
#endif // GAME
};