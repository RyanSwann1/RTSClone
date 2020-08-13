#pragma once

#include "glm/glm.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <random>

namespace Globals
{ 
	constexpr unsigned int INVALID_OPENGL_ID = 0;
	constexpr float GROUND_HEIGHT = 0.0f;
	constexpr int MAP_SIZE = 30;
	constexpr int NODE_SIZE = 6;

	constexpr int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};

	inline bool isPositionInMapBounds(const glm::ivec2& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE;
	}

	inline bool isPositionInMapBounds(const glm::vec3& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE * Globals::NODE_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE * Globals::NODE_SIZE &&
			position.z >= 0 &&
			position.z < Globals::MAP_SIZE * Globals::NODE_SIZE;
	}

	inline void print(const std::string& text)
	{
		static sf::Clock clock;

		if (clock.getElapsedTime().asSeconds() > 0.5f)
		{
			std::cout << text << "\n\n";
			clock.restart();
		}
	}

	inline void printImmediately(const glm::vec3& position)
	{
		std::cout << position.x << "\n";
		std::cout << position.y << "\n";
		std::cout << position.z << "\n\n";
	}

	inline void print(const glm::vec3& position)
	{
		static sf::Clock clock;
		
		if (clock.getElapsedTime().asSeconds() > 1.0f)
		{
			std::cout << position.x << "\n";
			std::cout << position.y << "\n";
			std::cout << position.z << "\n\n";
			clock.restart();
		}
	}

	//inline glm::vec3 convertToNearestNodePosition(const glm::vec3& position) 
	//{
	//	glm::vec3 newPosition = { std::floor(position.x), std::floor(position.y), std::floor(position.z) };
	//	int xDifference = static_cast<int>(position.x) % Globals::NODE_SIZE;
	//	if (xDifference > 0)
	//	{
	//		if (static_cast<float>(xDifference) >= static_cast<float>(NODE_SIZE) / 2.0f)
	//		{
	//			newPosition.x += NODE_SIZE - xDifference;
	//		}
	//		else
	//		{
	//			newPosition.x -= xDifference;
	//		}
	//	}

	//	int zDifference = static_cast<int>(position.z) % Globals::NODE_SIZE;
	//	if (zDifference > 0)
	//	{
	//		if (static_cast<float>(zDifference) >= static_cast<float>(NODE_SIZE) / 2.0f)
	//		{
	//			newPosition.z += NODE_SIZE - zDifference;
	//		}
	//		else
	//		{
	//			newPosition.z -= zDifference;
	//		}
	//	}

	//	return newPosition;
	//}

	inline glm::vec3 convertToNodePosition(const glm::vec3& position)
	{
		glm::vec3 newPosition = { std::floor(position.x), std::floor(position.y), std::floor(position.z) };
		int xDifference = static_cast<int>(position.x) % Globals::NODE_SIZE;
		if (xDifference > 0)
		{
			newPosition.x -= xDifference;
		}

		int zDifference = static_cast<int>(position.z) % Globals::NODE_SIZE;
		if (zDifference > 0)
		{
			newPosition.z -= zDifference;
		}

		return newPosition;
	}

	inline glm::vec3 convertToMiddlePosition(const glm::vec3& position)
	{
		return { position.x + Globals::NODE_SIZE / 2.0f, position.y, position.z + Globals::NODE_SIZE / 2.0f };
	}

	inline glm::vec3 convertToWorldPosition(const glm::ivec2& position)
	{
		return Globals::convertToMiddlePosition(
			{ position.x * Globals::NODE_SIZE, Globals::GROUND_HEIGHT, position.y * Globals::NODE_SIZE });
	}

	inline glm::ivec2 convertToGridPosition(const glm::vec3& position)
	{
		return { position.x / Globals::NODE_SIZE, position.z / Globals::NODE_SIZE };
	}

	inline int convertTo1D(const glm::ivec2& position)
	{
		return position.x * Globals::MAP_SIZE + position.y;
	}

	inline glm::vec3 moveTowards(const glm::vec3& currentPosition, const glm::vec3& targetPosition, float maxDistanceDelta)
	{
		float magnitude = glm::distance(targetPosition, currentPosition);
		if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
		{
			return targetPosition;
		}

		return currentPosition + glm::vec3(targetPosition - currentPosition) / magnitude * maxDistanceDelta;
	}

	inline float getRandomNumber(float min, float max)
	{
		static std::random_device rd;  //Will be used to obtain a seed for the random number engine
		static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> distrib(min, max);

		return distrib(gen);
	}
}