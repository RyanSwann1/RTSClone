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
#include "Headquarters.h"
#include "SelectionBox.h"
#include "Map.h"

#define RENDER_GROUND
#ifdef RENDER_GROUND
#include "Ground.h"
#endif // RENDER_GROUND

//OpenGL Debug
//https://gist.github.com/qookei/76586d33238f0fa918c499dc7fb5ed04
//A*
//https://www.youtube.com/watch?v=icZj67PTFhc
//Pathfinding Article
//http://striketactics.net/devblog/starcraft-1-pathfinding-technical-analysis
//Pathfinding Playlist
//https://www.youtube.com/playlist?list=PLFt_AvWsXl0cq5Umv3pMC9SPnKjfp9eGW
//Smooth Pathfinding
//https://gamedev.stackexchange.com/questions/42106/2d-pathfinding-finding-smooth-paths
//Design Patterns
//https://www.youtube.com/watch?v=hQE8lQk9ikE

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

	std::unique_ptr<Model> spacecraftModel = Model::create("spaceCraft1.obj", false, glm::vec3(5.0f, 0.25f, 5.0f));
	assert(spacecraftModel);
	if (!spacecraftModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		return -1;
	}

	std::unique_ptr<Model> headquartersModel = Model::create("portal.obj", true, glm::vec3(5.0f, 0.25f, 3.0f));
	assert(headquartersModel);
	if (!headquartersModel)
	{
		std::cout << "Failed to load portal model\n";
		return -1;
	}

	std::unique_ptr<Model> rocksOreModel = Model::create("rocksOre.obj", true, glm::vec3(5.0f, 0.25f, 5.0f));
	assert(rocksOreModel);
	if (!rocksOreModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		return -1;
	}

	std::unique_ptr<Model> waypointModel = Model::create("laserSabel.obj", true, glm::vec3(2.0f, 1.0f, 2.0f));
	assert(waypointModel);
	if (!waypointModel)
	{
		std::cout << "Failed to load laserSabel Model\n";
		return -1;
	}

	Map map;
#ifdef RENDER_GROUND
	Ground ground;
#endif // RENDER_GROUND
	SelectionBox selectionBox;
	sf::Clock gameClock;
	Camera camera;
	Entity waypoint({ 50.0f, Globals::GROUND_HEIGHT, 50.0f }, *waypointModel);
	Entity mineral({ 10.0, Globals::GROUND_HEIGHT, 10.0f }, *rocksOreModel);
	Unit spacecraft({ 20.0f, Globals::GROUND_HEIGHT, 20.0f }, *spacecraftModel);
	Headquarters headquarters({ 37.5f, Globals::GROUND_HEIGHT, 37.5f }, *headquartersModel);

	map.addEntityAABB(headquarters.getAABB());
	map.addEntityAABB(mineral.getAABB());

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
			selectionBox.update(projection, view, camera, window, spacecraft, headquarters);
			selectionBox.handleInputEvents(currentSFMLEvent, window, projection, view, camera, spacecraft, map);
		}
		
		spacecraft.update(deltaTime);
		camera.update(window, deltaTime);

		glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(camera.FOV),
			static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), camera.nearPlaneDistance, camera.farPlaneDistance);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);
	
		spacecraft.render(*shaderHandler, *spacecraftModel);
		headquarters.render(*shaderHandler, *headquartersModel);
		mineral.render(*shaderHandler, *rocksOreModel);
		waypoint.render(*shaderHandler, *waypointModel);

		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

#ifdef RENDER_GROUND
		ground.render(*shaderHandler);
#endif // RENDER_GROUND

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef RENDER_AABB
		headquarters.renderAABB(*shaderHandler);
		spacecraft.renderAABB(*shaderHandler);
		mineral.renderAABB(*shaderHandler);
#endif // RENDER_AABB
#ifdef RENDER_PATHING
		spacecraft.renderPathMesh(*shaderHandler);
#endif // RENDER_PATHING
		shaderHandler->switchToShader(eShaderType::SelectionBox);
		selectionBox.render(window);
		glDisable(GL_BLEND);

		window.display();
	}

	return 0;
}