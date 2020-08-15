#include <iostream>

#include "glad.h"
#include <SFML/Graphics.hpp>
#include "ModelLoader.h"
#include "ShaderHandler.h"
#include "Texture.h"
#include "Camera.h"
#include "Unit.h"
#include "Headquarters.h"
#include "Map.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Faction.h"
#include "ModelManager.h"
#include "Mineral.h"
#include "imgui/imgui.h"
#include "imgui_impl/imgui_wrapper.h"

#define RENDER_GROUND
#ifdef RENDER_GROUND
#include "Ground.h"
#endif // RENDER_GROUND

//Networking Protocol
//https://gamedev.stackexchange.com/a/162509/76861
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
//Game engine from scratch
//https://www.gamasutra.com/blogs/MichaelKissner/20151027/257369/Writing_a_Game_Engine_from_Scratch__Part_1_Messaging.php

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
	//glm::uvec2 windowSize(1980, 1080);
	sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Default, settings);
	//sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Fullscreen, settings);
	window.setFramerateLimit(60);
	window.setMouseCursorGrabbed(true);
	gladLoadGL();

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);
	ImGui_SFML_OpenGL3::init(window);
	
	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	std::unique_ptr<ModelManager> modelManager = ModelManager::create();
	assert(modelManager);
	if (!modelManager)
	{
		std::cout << "Failed to load all models\n";
		return -1;
	}

#ifdef RENDER_GROUND
	Ground ground;
#endif // RENDER_GROUND
	std::unique_ptr<Map> map = std::make_unique<Map>();
	Faction faction(*modelManager, *map);
	sf::Clock gameClock;
	Camera camera;
	std::vector<Mineral> minerals;

	for (float z = Globals::NODE_SIZE; z <= Globals::NODE_SIZE * 5; z += Globals::NODE_SIZE)
	{
		minerals.emplace_back(Globals::convertToNodePosition({ 70.0f, Globals::GROUND_HEIGHT, z }),
			modelManager->getModel(eModelName::Mineral), *map);
	}

	shaderHandler->switchToShader(eShaderType::SelectionBox);
	shaderHandler->setUniformMat4f(eShaderType::SelectionBox, "uOrthographic", glm::ortho(0.0f, static_cast<float>(windowSize.x),
		static_cast<float>(windowSize.y), 0.0f));

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

			faction.handleInput(currentSFMLEvent, window, camera, *map, *modelManager, minerals, deltaTime);
		}

		ImGui_SFML_OpenGL3::startFrame();

		ImGui::ShowDemoWindow();

		faction.update(deltaTime, *modelManager, *map);
		camera.update(window, deltaTime);

		glm::mat4 view = camera.getView(); 
		glm::mat4 projection = camera.getProjection(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);
	
		faction.render(*shaderHandler, *modelManager);
		for(const auto& mineral : minerals)
		{
			mineral.render(*shaderHandler, modelManager->getModel(mineral.getModelName()));
		}

		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

#ifdef RENDER_GROUND
		ground.render(*shaderHandler);
#endif // RENDER_GROUND

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

#ifdef RENDER_AABB
		faction.renderAABB(*shaderHandler);
		for (auto& mineral : minerals)
		{
			mineral.renderAABB(*shaderHandler);
		}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
		faction.renderPathing(*shaderHandler);
#endif // RENDER_PATHING
		
		shaderHandler->switchToShader(eShaderType::SelectionBox);
		faction.renderSelectionBox(window);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		ImGui::EndFrame();
		window.display();
	}

	ImGui_SFML_OpenGL3::shutdown();
	return 0;
}