#pragma once

#include "SelectionBox.h"
#include "glm/glm.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace Globals
{ 
	constexpr unsigned int INVALID_OPENGL_ID = 0;
	constexpr float GROUND_HEIGHT = 0.0f;
	constexpr int MAP_SIZE = 75;
	constexpr int NODE_SIZE = 1;

	inline void print(const std::string& text)
	{
		static sf::Clock clock;

		if (clock.getElapsedTime().asSeconds() > 0.5f)
		{
			std::cout << text << "\n\n";
			clock.restart();
		}
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
}