#pragma once

#include "SelectionBox.h"
#include "glm/glm.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

namespace Globals
{ 
	constexpr unsigned int INVALID_OPENGL_ID = 0;
	constexpr float GROUND_HEIGHT = 0.0f;
	constexpr glm::ivec2 MAP_SIZE = { 50, 50 };


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
}