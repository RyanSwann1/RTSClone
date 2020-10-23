#include "Camera.h"
#include "Globals.h"
#include "glm/gtc/matrix_transform.hpp"

namespace
{
#ifdef LEVEL_EDITOR
	constexpr float MOVEMENT_SPEED = 15.0f;
	constexpr float ZOOM_SPEED = 30.0f;
#endif // LEVEL_EDITOR

#ifdef GAME
	constexpr float MOVEMENT_SPEED = 110.0f;
#endif // GAME

	constexpr int MAX_RAY_TO_GROUND_DISTANCE = 2500;
	constexpr float MINIMUM_HEIGHT = 5.0f;
	constexpr float SENSITIVITY = 4.0f;
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

	constexpr float ZOOM_STEP = 7.5f;
	constexpr float MAXIMUM_ZOOM_HEIGHT = 25.0f;
	constexpr float MINIMUM_ZOOM_HEIGHT = 150.0f;
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
	position(STARTING_POSITION),
	velocity()
{
	setFront();
	right = glm::normalize(glm::cross(front, up));
	up = glm::normalize(glm::cross(right, front));
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

#ifdef GAME
void Camera::update(float deltaTime)
{
	setFront();
	moveByArrowKeys(deltaTime);
}

glm::vec3 Camera::getMouseToGroundPosition(const sf::Window& window) const
{
	assert(position.y >= MINIMUM_HEIGHT);

	glm::vec3 rayPositionFromMouse = calculateMouseRay(getProjection(window), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	glm::vec3 rayPosition = position;
	int i = static_cast<int>(position.y);
	while (rayPosition.y > 0.0f)
	{
		rayPosition = rayPositionFromMouse * static_cast<float>(i) + position;
		++i;
	}

	return { rayPosition.x, Globals::GROUND_HEIGHT, rayPosition.z };
}
#endif // GAME

#ifdef LEVEL_EDITOR
bool Camera::getMouseToGroundPosition(const sf::Window& window, glm::vec3& mouseToGroundPosition) const
{
	assert(position.y >= MINIMUM_HEIGHT);

	glm::vec3 rayPositionFromMouse = calculateMouseRay(getProjection(window), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	glm::vec3 rayPosition = position;
	bool groundFound = false;
	for (int i = 1; i < MAX_RAY_TO_GROUND_DISTANCE; ++i)
	{
		rayPosition = rayPositionFromMouse * static_cast<float>(i) + position;
		if (rayPosition.y <= 0.0f)
		{
			groundFound = true;
			mouseToGroundPosition = { rayPosition.x, Globals::GROUND_HEIGHT, rayPosition.z };
			break;
		}
	}

	return groundFound;
}

void Camera::onMouseMove(const sf::Window& window, float deltaTime)
{
	if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
	{
		rotation.x += (static_cast<int>(window.getSize().y / 2) - sf::Mouse::getPosition(window).y) * sensitivity * deltaTime;
		rotation.y += (sf::Mouse::getPosition(window).x - static_cast<int>(window.getSize().x / 2)) * sensitivity * deltaTime;

		sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);

		setFront();
		right = glm::normalize(glm::cross(front, { 0.0f, 1.0f, 0.0f }));
		up = glm::normalize(glm::cross(right, front));
	}
}

void Camera::update(float deltaTime, const sf::Window& window, glm::ivec2 lastMousePosition)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
	{
		zoom(window, lastMousePosition);
	}
	else
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			velocity += front * MOVEMENT_SPEED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			velocity -= front * MOVEMENT_SPEED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			velocity += right * MOVEMENT_SPEED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			velocity -= right * MOVEMENT_SPEED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			velocity += up * MOVEMENT_SPEED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{
			velocity -= up * MOVEMENT_SPEED;
		}	
	}

	
	glm::vec3 newPosition = position + velocity * deltaTime;
	if (newPosition.y >= MINIMUM_HEIGHT)
	{
		position = newPosition;
	}

	velocity *= 0.9f;
	setFront();
	right = glm::normalize(glm::cross(front, { 0.0f, 1.0f, 0.0f }));
	up = glm::normalize(glm::cross(right, front));
	if (glm::abs(velocity.x) <= 0.2f)
	{
		velocity.x = 0.0f;
	}
	if (glm::abs(velocity.y) <= 0.2f)
	{
		velocity.y = 0.0f;
	}
	if (glm::abs(velocity.z) <= 0.2f)
	{
		velocity.z = 0.0f;
	}
}

void Camera::zoom(int mouseWheelDelta)
{
	glm::vec3 newPosition = { 0.0f, 0.0f, 0.0f };
	if (mouseWheelDelta > 0)
	{
		newPosition = position + glm::normalize(front) * ZOOM_STEP;
		if (newPosition.y >= MAXIMUM_ZOOM_HEIGHT)
		{
			position = newPosition;
		}
	}
	else
	{
		newPosition = position - glm::normalize(front) * ZOOM_STEP;
		if (newPosition.y <= MINIMUM_ZOOM_HEIGHT)
		{
			position = newPosition;
		}
	}
}

void Camera::zoom(const sf::Window& window, glm::ivec2 lastMousePosition)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
	{
		if (sf::Mouse::getPosition(window).x > lastMousePosition.x)
		{
			velocity += front * ZOOM_SPEED;
			sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
		}
		else if (sf::Mouse::getPosition(window).x < lastMousePosition.x)
		{
			velocity -= front * ZOOM_SPEED;
			sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
		}
	}
}
#endif // LEVEL_EDITOR

void Camera::moveByArrowKeys(float deltaTime)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		position -= glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		position += glm::normalize(glm::cross(front, up)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		position.x += glm::cos(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
		position.z += glm::sin(glm::radians(rotation.y)) * MOVEMENT_SPEED * deltaTime;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
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