#include "Camera.h"
#include "Globals.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/projection.hpp"
#include <limits>

namespace
{
#ifdef LEVEL_EDITOR
	const float MOVEMENT_SPEED = 15.0f;
	const float ZOOM_SPEED = 30.0f;
#endif // LEVEL_EDITOR

#ifdef GAME
	const float MOVEMENT_SPEED = 110.0f;
	const float MOUSE_MOVEMENT_SPEED = 90.0f;
#endif // GAME

	const float SENSITIVITY = 4.0f;
	const float NEAR_PLANE_DISTANCE = 0.1f;
	const float FAR_PLANE_DISTANCE = 1750.0f;
	const float FIELD_OF_VIEW = 50.0f;
	const float MOUSE_MOVE_BOUNDARY = 0.95f;
	const glm::vec3 STARTING_POSITION = { 0.0f, 72.0f, 43.0f };
	const glm::vec3 STARTING_ROTATION = { -70.0f, 0.0f, 0.0f };

	const float ZOOM_STEP = 7.5f;
	const float MAXIMUM_ZOOM_HEIGHT = 25.0f;
	const float MINIMUM_ZOOM_HEIGHT = 150.0f;

	//RAY TO AABB http://www.3dkingdoms.com/weekly/weekly.php?a=3
	glm::vec3 getRayDirectionFromCamera(const glm::mat4& projection, const glm::mat4& view, const sf::Window& window, glm::ivec2 mousePosition)
	{
		glm::vec4 clipSpace = { (mousePosition.x * 2.0f) / window.getSize().x - 1.0f,
			1.0f - (mousePosition.y * 2.0f) / window.getSize().y,
			-1.0f,
			1.0f };
		glm::vec4 viewSpace = glm::inverse(projection) * clipSpace;
		glm::vec3 rayDirection = glm::inverse(view) * glm::vec4(viewSpace.x, viewSpace.y, -1.0f, 0.0f);

		return glm::normalize(rayDirection);
	}

	int GetIntersection(float fDst1, float fDst2, const glm::vec3& P1, const glm::vec3& P2, glm::vec3& Hit) {
		if ((fDst1 * fDst2) >= 0.0f) return 0;
		if (fDst1 == fDst2) return 0;
		Hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
		return 1;
	}

	int InBox(const glm::vec3& Hit, const glm::vec3& B1, const glm::vec3& B2, const int Axis) {
		if (Axis == 1 && Hit.z > B1.z && Hit.z < B2.z && Hit.y > B1.y && Hit.y < B2.y) return 1;
		if (Axis == 2 && Hit.z > B1.z && Hit.z < B2.z && Hit.x > B1.x && Hit.x < B2.x) return 1;
		if (Axis == 3 && Hit.x > B1.x && Hit.x < B2.x && Hit.y > B1.y && Hit.y < B2.y) return 1;
		return 0;
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

glm::mat4 Camera::getProjection(glm::ivec2 windowSize) const
{
	return glm::perspective(glm::radians(FOV),
		static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), nearPlaneDistance, farPlaneDistance);
}

#ifdef GAME
void Camera::update(float deltaTime, const sf::Window& window, glm::uvec2 windowSize)
{
	setFront();
	moveByArrowKeys(deltaTime);
	moveByMouse(deltaTime, window, windowSize);
}

glm::vec3 Camera::getInfiniteForwardRay(const sf::Window& window) const
{
	glm::vec3 rayStartingPosition = position;
	glm::vec3 rayDirection = getRayDirectionFromCamera(getProjection(glm::ivec2(window.getSize().x, window.getSize().y)), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	return rayStartingPosition + rayDirection * std::numeric_limits<float>::max();
}

void Camera::moveByMouse(float deltaTime, const sf::Window& window, glm::uvec2 windowSize)
{
	glm::vec2 mousePosition = { static_cast<float>(sf::Mouse::getPosition(window).x), static_cast<float>(sf::Mouse::getPosition(window).y) };
	glm::vec2 mousePositionNDC = { (mousePosition.x / windowSize.x * 2.0f) - 1.0f, (mousePosition.y / windowSize.y * 2.0f) - 1.0f };

	if (mousePositionNDC.x <= -MOUSE_MOVE_BOUNDARY)
	{
		position -= glm::normalize(glm::cross(front, up)) * MOUSE_MOVEMENT_SPEED * deltaTime;
	}
	if (mousePositionNDC.x >= MOUSE_MOVE_BOUNDARY)
	{
		position += glm::normalize(glm::cross(front, up)) * MOUSE_MOVEMENT_SPEED * deltaTime;
	}
	if (mousePositionNDC.y <= -MOUSE_MOVE_BOUNDARY)
	{
		position.x += glm::cos(glm::radians(rotation.y)) * MOUSE_MOVEMENT_SPEED * deltaTime;
		position.z += glm::sin(glm::radians(rotation.y)) * MOUSE_MOVEMENT_SPEED * deltaTime;
	}
	if (mousePositionNDC.y >= MOUSE_MOVE_BOUNDARY)
	{
		position.x -= glm::cos(glm::radians(rotation.y)) * MOUSE_MOVEMENT_SPEED * deltaTime;
		position.z -= glm::sin(glm::radians(rotation.y)) * MOUSE_MOVEMENT_SPEED * deltaTime;
	}
}

glm::vec3 Camera::getRayToGroundPlaneIntersection(const sf::Window& window) const
{
	glm::vec3 planeNormal = { 0.0f, 1.0f, 0.0f };
	glm::vec3 rayStartingPosition = position;
	glm::vec3 rayDirection = getRayDirectionFromCamera(getProjection(glm::ivec2(window.getSize().x, window.getSize().y)), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	float k = glm::dot(glm::proj(-rayStartingPosition, planeNormal), planeNormal) / glm::dot(glm::proj(rayDirection, planeNormal), planeNormal);
	glm::vec3 intersection = (rayStartingPosition + rayDirection * k);

	assert(k >= 0.0f);
	return intersection;
}
#endif // GAME

#ifdef LEVEL_EDITOR
bool Camera::isRayIntersectingAABB(const glm::vec3& B1, const glm::vec3& B2, const glm::vec3& L1, const glm::vec3& L2, glm::vec3& Hit) const
{
	if (L2.x < B1.x && L1.x < B1.x) return false;
	if (L2.x > B2.x && L1.x > B2.x) return false;
	if (L2.y < B1.y && L1.y < B1.y) return false;
	if (L2.y > B2.y && L1.y > B2.y) return false;
	if (L2.z < B1.z && L1.z < B1.z) return false;
	if (L2.z > B2.z && L1.z > B2.z) return false;
	if (L1.x > B1.x && L1.x < B2.x &&
		L1.y > B1.y && L1.y < B2.y &&
		L1.z > B1.z && L1.z < B2.z)
	{
		Hit = L1;
		return true;
	}
	if ((GetIntersection(L1.x - B1.x, L2.x - B1.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
		|| (GetIntersection(L1.y - B1.y, L2.y - B1.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
		|| (GetIntersection(L1.z - B1.z, L2.z - B1.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3))
		|| (GetIntersection(L1.x - B2.x, L2.x - B2.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
		|| (GetIntersection(L1.y - B2.y, L2.y - B2.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
		|| (GetIntersection(L1.z - B2.z, L2.z - B2.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3)))
		return true;

	return false;
}

glm::vec3 Camera::getRayDirectionFromMouse(const sf::Window& window) const
{
	return getRayDirectionFromCamera(getProjection(glm::ivec2(window.getSize().x, window.getSize().y)), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });
}

bool Camera::getRayToGroundIntersection(const sf::Window& window, glm::uvec2 windowSize, glm::vec3& intersection) const
{
	glm::vec3 planeNormal = { 0.0f, 1.0f, 0.0f };
	glm::vec3 rayStartingPosition = position;
	glm::vec3 rayDirection = getRayDirectionFromCamera(getProjection(glm::ivec2(window.getSize().x, window.getSize().y)), getView(), window,
		{ sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y });

	if (glm::dot(rayDirection, planeNormal) > -std::numeric_limits<float>::epsilon())
	{
		return false;
	}

	float v = glm::dot(glm::proj(-rayStartingPosition, planeNormal), planeNormal) / glm::dot(glm::proj(rayDirection, planeNormal), planeNormal);
	intersection = (rayStartingPosition + rayDirection * v);

	return true;
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

	position += velocity * deltaTime;

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