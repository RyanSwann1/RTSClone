#include "Camera.h"
#include "Globals.h"
#include "glm/gtc/matrix_transform.hpp"

namespace
{
	constexpr float MAX_RAY_TO_GROUND_DISTANCE = 1500.0f;
	constexpr float MOVEMENT_SPEED = 110.0f;
	constexpr float SENSITIVITY = 0.1f;
	constexpr float NEAR_PLANE_DISTANCE = 0.1f;
	constexpr float FAR_PLANE_DISTANCE = 1750.0f;
	constexpr float FIELD_OF_VIEW = 50.0f;
	constexpr glm::vec3 STARTING_POSITION = { 0.0f, 72.0f, 43.0f };
	constexpr glm::vec3 STARTING_ROTATION = { -75.0f, 0.0f, 0.0f };

	glm::vec2 getNormalizedDeviceCoords(const sf::Window& window, const glm::ivec2& mousePosition)
	{
		float x = (2.0f * mousePosition.x) / window.getSize().x - 1.0f;
		float y = 1.0f - (2.0f * mousePosition.y) / window.getSize().y;

		return glm::vec2(x, y);
	}

	glm::vec4 toEyeCoords(const glm::vec4& clipSpaceCoords, const glm::mat4& projection)
	{
		glm::vec4 rayEye = glm::inverse(projection) * clipSpaceCoords;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		return rayEye;
	}

	glm::vec3 toWorldCoords(const glm::mat4& view, const glm::vec4& eyeCoords)
	{
		glm::vec4 position = glm::vec4(glm::inverse(view) * eyeCoords);
		glm::vec3 rayWorld = { position.x, position.y, position.z };
		return glm::normalize(rayWorld);
	}

	glm::vec3 calculateMouseRay(const glm::mat4& projection, const glm::mat4& view, const sf::Window& window,
		const glm::ivec2& mousePosition)
	{
		glm::vec2 normalizedMouseCoords = getNormalizedDeviceCoords(window, mousePosition);
		glm::vec4 clipCoords = glm::vec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0f, 1.0f);
		glm::vec4 eyeCoords = toEyeCoords(clipCoords, projection);
		glm::vec3 worldRay = toWorldCoords(view, eyeCoords);

		return worldRay;
	}
}

Camera::Camera()
	: FOV(FIELD_OF_VIEW),
	sensitivity(SENSITIVITY),
	nearPlaneDistance(NEAR_PLANE_DISTANCE),
	farPlaneDistance(FAR_PLANE_DISTANCE),
	front(),
	up({ 0.0f, 1.0f, 0.0f }),
	right(),
	rotation(STARTING_ROTATION),
	position(STARTING_POSITION)
{
	setFront();
	right = glm::normalize(glm::cross(front, up));
}

glm::mat4 Camera::getView() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjection(const sf::Window& window) const
{
	return glm::perspective(glm::radians(FOV),
		static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y), nearPlaneDistance, farPlaneDistance);
}

glm::vec3 Camera::getMouseToGroundPosition(const sf::Window& window) const
{
	assert(position.y > 0.0f);

	glm::vec3 rayPositionFromMouse = calculateMouseRay(getProjection(window), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	glm::vec3 rayPosition = position;
	int i = position.y;
	while (rayPosition.y > 0.0f)
	{
		rayPosition = rayPositionFromMouse * static_cast<float>(i) + position;
		++i;
	}

	return { rayPosition.x, Globals::GROUND_HEIGHT, rayPosition.z };
}

void Camera::update(float deltaTime)
{
	moveByArrowKeys(deltaTime);
	setFront();
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

void Camera::setFront()
{
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