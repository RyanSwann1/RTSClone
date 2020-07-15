#include "glm/glm.hpp"
#include "glad.h"
#include "Camera.h"
#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = sf::ContextSettings::Core;
	glm::uvec2 windowSize(640, 480);
	sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Default, settings);
	window.setFramerateLimit(60);
	gladLoadGL();
	glViewport(0, 0, windowSize.x, windowSize.y);

	Camera camera;

	while (window.isOpen())
	{
		sf::Event currentSFMLEvent;
		while (window.pollEvent(currentSFMLEvent))
		{
			if (currentSFMLEvent.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.update(window);

		window.display();
	}

	return 0;
}