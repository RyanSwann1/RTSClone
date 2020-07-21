#include "Camera.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 1.0f;
	constexpr float SENSITIVITY = 0.1f;
	constexpr float NEAR_PLANE_DISTANCE = 0.1f;
	constexpr float FAR_PLANE_DISTANCE = 1750.0f;
	constexpr float FIELD_OF_VIEW = 50.0f;
}

Camera::Camera()
	: FOV(FIELD_OF_VIEW),
	sensitivity(SENSITIVITY),
	nearPlaneDistance(NEAR_PLANE_DISTANCE),
	farPlaneDistance(FAR_PLANE_DISTANCE),
	front(),
	right(),
	up(),
	rotation(0.f, 0.f),
	position(0, 0, 0)
{
	front = { 0.0f, -1.0f, 0.0f };
	right = glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, front));
	up = { 0.0f, 1.0f, 0.0f };
}

void Camera::update(const sf::Window& window)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		position += front * MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		position -= front * MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		position -= glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		position += glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		position.y += MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
	{
		position.y -= MOVEMENT_SPEED;
	}

	rotation.x += (static_cast<int>(window.getSize().y / 2) - sf::Mouse::getPosition(window).y) * sensitivity;
	rotation.y += (sf::Mouse::getPosition(window).x - static_cast<int>(window.getSize().x / 2)) * sensitivity;

	sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (rotation.x > 89.0f)
		rotation.x = 89.0f;
	if (rotation.x < -89.0f)
		rotation.x = -89.0f;

	glm::vec3 v = {
		glm::cos(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)),
		glm::sin(glm::radians(rotation.x)),
		glm::sin(glm::radians(rotation.y)) * glm::cos(glm::radians(rotation.x)) };
	front = glm::normalize(v);
}