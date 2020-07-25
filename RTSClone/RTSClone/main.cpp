#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "glad.h"
#include <SFML/Graphics.hpp>
#include "Model.h"
#include "ModelLoader.h"
#include "ShaderHandler.h"
#include "Texture.h"
#include "Camera.h"
#include "Unit.h"
#include "SelectionBox.h"
#include "Ground.h"
#include "Globals.h"

//OpenGL Debug
//https://gist.github.com/qookei/76586d33238f0fa918c499dc7fb5ed04
//A*
//https://www.youtube.com/watch?v=icZj67PTFhc

//Pathfinding Playlist
//https://www.youtube.com/playlist?list=PLFt_AvWsXl0cq5Umv3pMC9SPnKjfp9eGW

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.attributeFlags = sf::ContextSettings::Core;
	glm::uvec2 windowSize(1280, 800);
	sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Default, settings);
	window.setFramerateLimit(60);
	window.setMouseCursorGrabbed(true);
	gladLoadGL();

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);

	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	std::unique_ptr<Model> spacecraftModel = Model::create("models/spaceCraft1.obj");
	assert(spacecraftModel);
	if (!spacecraftModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		return -1;
	}

	Ground ground;
	SelectionBox selectionBox;
	sf::Clock gameClock;
	Camera camera;
	glm::vec3 startingPosition = { 0.0f, Globals::GROUND_HEIGHT, 0.0f };
	std::vector<Unit> units;
	for (int i = 0; i < 20; ++i)
	{
		units.emplace_back(glm::vec3(startingPosition.x, startingPosition.y, startingPosition.z));
		startingPosition.z += 12.5f;
		startingPosition.x += 12.5f;
	}


	glm::mat4 orthographic = glm::ortho(0.0f, static_cast<float>(windowSize.x),
		static_cast<float>(windowSize.y), 0.0f);
	shaderHandler->switchToShader(eShaderType::SelectionBox);
	shaderHandler->setUniformMat4f(eShaderType::SelectionBox, "uOrthographic", orthographic);

	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";

	while (window.isOpen())
	{
		float deltaTime = gameClock.restart().asSeconds();
		sf::Event currentSFMLEvent;
		while (window.pollEvent(currentSFMLEvent))
		{
			if (currentSFMLEvent.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (currentSFMLEvent.type == sf::Event::KeyPressed)
			{
				if (currentSFMLEvent.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}
			}

			glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
			glm::mat4 projection = glm::perspective(glm::radians(camera.FOV),
				static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), camera.nearPlaneDistance, camera.farPlaneDistance);
			selectionBox.update(projection, view, camera, window, units);
			selectionBox.handleInputEvents(currentSFMLEvent, window, projection, view, camera);
		}
		
		camera.update(window, deltaTime);

		glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(camera.FOV),
			static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), camera.nearPlaneDistance, camera.farPlaneDistance);
		selectionBox.update(projection, view, camera, window, units);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);

		for (auto& unit : units)
		{			
			unit.render(*shaderHandler, *spacecraftModel);
		}

		shaderHandler->switchToShader(eShaderType::Ground);
		shaderHandler->setUniformMat4f(eShaderType::Ground, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Ground, "uProjection", projection);
		ground.render();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shaderHandler->switchToShader(eShaderType::SelectionBox);
		selectionBox.render(window);
		glDisable(GL_BLEND);

		window.display();
	}

	return 0;
}