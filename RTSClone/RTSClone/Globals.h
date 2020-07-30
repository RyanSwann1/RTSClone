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
	constexpr int MAP_SIZE = 75;
	constexpr int NODE_SIZE = 1;

	constexpr int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};

	inline bool isPositionInMapBounds(const glm::vec3& position)
	{
		return position.x >= 0 &&
			position.x < Globals::MAP_SIZE &&
			position.y >= 0 &&
			position.y < Globals::MAP_SIZE &&
			position.z >= 0 &&
			position.z < Globals::MAP_SIZE;
		
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

	inline int convertTo1D(const glm::ivec2& position)
	{
		return position.x * Globals::MAP_SIZE + position.y;
	}

	inline float getRandomNumber(float min, float max)
	{
		static std::random_device rd;  //Will be used to obtain a seed for the random number engine
		static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<> distrib(min, max);

		return distrib(gen);
	}
}