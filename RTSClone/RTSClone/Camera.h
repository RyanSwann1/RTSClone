#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include <SFML/Graphics.hpp>

struct Camera : private NonCopyable, private NonMovable
{
	Camera();

	void update(const sf::Window& window);

	const float FOV;
	const float sensitivity;
	const float nearPlaneDistance;
	const float farPlaneDistance;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec2 rotation;
	glm::vec3 position;
};