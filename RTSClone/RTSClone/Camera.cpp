#include "Camera.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 20.0f;
	constexpr float SENSITIVITY = 0.1f;
	constexpr float NEAR_PLANE_DISTANCE = 0.1f;
	constexpr float FAR_PLANE_DISTANCE = 1750.0f;
	constexpr float FIELD_OF_VIEW = 50.0f;
	constexpr glm::vec3 STARTING_POSITION = { 0.0f, 50.0f, -55.0f };
	constexpr glm::vec3 STARTING_ROTATION = { -35.0f, 89.0f, 0.0f };

	
}

Camera::Camera()
	: FOV(FIELD_OF_VIEW),
	sensitivity(SENSITIVITY),
	nearPlaneDistance(NEAR_PLANE_DISTANCE),
	farPlaneDistance(FAR_PLANE_DISTANCE),
	front(),
	right(),
	up(),
	rotation(STARTING_ROTATION),
	position(STARTING_POSITION)
{
	front = { 0.0f, -1.0f, 0.0f };
	right = glm::normalize(glm::cross({ 0.0f, 1.0f, 0.0f }, front));
	up = { 0.0f, 1.0f, 0.0f };
}

void Camera::update(const sf::Window& window, float deltaTime)
{

	moveByArrowKeys(deltaTime);

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

	//rotation.x += (static_cast<int>(window.getSize().y / 2) - sf::Mouse::getPosition(window).y) * sensitivity;
	//rotation.y += (sf::Mouse::getPosition(window).x - static_cast<int>(window.getSize().x / 2)) * sensitivity;

	//sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);

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

void Camera::moveByArrowKeys(float deltaTime)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		position -= glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		position += glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		position.x += glm::cos(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
		position.z += glm::sin(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		position.x -= glm::cos(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
		position.z -= glm::sin(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
	}
}
