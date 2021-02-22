#include <iostream>

#include "glad.h"
#include <SFML/Graphics.hpp>
#include "ModelLoader.h"
#include "ShaderHandler.h"
#include "Camera.h"
#include "Unit.h"
#include "Map.h"
#include "glm/gtc/matrix_transform.hpp"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "ModelManager.h"
#include "Mineral.h"
#include "GameEventHandler.h"
#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>
#include "ProjectileHandler.h"
#include "Level.h"
#include "UIManager.h"
#include "GameMessenger.h"
#include "PathFinding.h"
#include "LevelFileHandler.h"

//AI
//https://www.youtube.com/watch?v=V3qASwCM-PE&list=PLdgLYFdStKu03Dv9GUsXBDQMdyJbkDb8i
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
	glm::uvec2 windowSize = Globals::WINDOW_SIZE;
	//glm::uvec2 windowSize(1980, 1080);
	sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Default, settings);
	//sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "RTS Clone", sf::Style::Fullscreen, settings);
	window.setFramerateLimit(60);
	window.setMouseCursorGrabbed(true);
	gladLoadGL();

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BACK);
	ImGui_SFML_OpenGL3::init(window);
	
	if (!ModelManager::getInstance().isAllModelsLoaded())
	{
		std::cout << "Failed to load all models\n";
		return -1;
	}

	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	GameMessenger<GameMessages::UIClearDisplaySelectedEntity>::getInstance();
	GameMessenger<GameMessages::UIClearWinner>::getInstance();
	GameMessenger<GameMessages::AddToMap>::getInstance();
	GameMessenger<GameMessages::RemoveFromMap>::getInstance();
	GameMessenger<GameMessages::NewMapSize>::getInstance();
	GameMessenger<GameMessages::UIDisplayPlayerDetails>::getInstance();
	GameMessenger<GameMessages::UIDisplaySelectedEntity>::getInstance();
	GameMessenger<GameMessages::UIDisplayWinner>::getInstance();

	PathFinding::getInstance();
	
	sf::Clock gameClock;
	Camera camera;
	UIManager uiManager;
	Map map;
	const std::array<std::string, Globals::MAX_LEVELS> levelNames = LevelFileHandler::loadLevelNames();
	std::unique_ptr<Level> level; 

	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";

	while (window.isOpen())
	{
		float deltaTime = gameClock.restart().asSeconds();
		//Handle Input
		sf::Event currentSFMLEvent;
		while (window.pollEvent(currentSFMLEvent))
		{
			switch (currentSFMLEvent.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (currentSFMLEvent.key.code == sf::Keyboard::Escape)
				{
					(level ? level.reset() : window.close());
				}
				break;
			}

			if (level)
			{
				level->handleInput(window, camera, currentSFMLEvent, map, uiManager);
			}
		}

		ImGui_SFML_OpenGL3::startFrame();
		//ImGui::ShowDemoWindow();

		if (level)
		{
			const Faction* winningFaction = level->getWinningFaction();
			if (winningFaction)
			{
				broadcastToMessenger<GameMessages::UIDisplayWinner>({ winningFaction->getController() });
				level.reset();
			}
		}
		else
		{
			ImGui::Begin("Level Selection");
			for (const auto& levelName : levelNames)
			{
				if (!levelName.empty() && ImGui::Button(levelName.c_str()))
				{
					broadcastToMessenger<GameMessages::UIClearWinner>({});
					level = Level::create(levelName, camera);
					assert(level);
					if (!level)
					{
						std::cout << "Unable to load " << levelName << "\n";
					}
					break;
				}
			}

			ImGui::End();
		}
		
		//Update
		uiManager.render(window);
		if (level)
		{
			camera.update(deltaTime, window, windowSize);
			level->update(deltaTime, map, uiManager);
		}

		//Render
		glm::mat4 view = camera.getView(); 
		glm::mat4 projection = camera.getProjection(glm::ivec2(window.getSize().x, window.getSize().y));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);
		shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 1.0f);
	
		if (level)
		{
			level->render(*shaderHandler);
		}

		glDisable(GL_CULL_FACE);
		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

		if (level)
		{
			level->renderTerrain(*shaderHandler);
		}
		glEnable(GL_CULL_FACE);
		glEnable(GL_BACK);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
#ifdef RENDER_AABB
		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);
		if (level)
		{
			level->renderAABB(*shaderHandler);
		}
#endif // RENDER_AABB

#ifdef RENDER_PATHING
		if (level)
		{
			level->renderPathing(*shaderHandler);
		}
#endif // RENDER_PATHING

		if (level)
		{
			glDisable(GL_CULL_FACE);
			shaderHandler->switchToShader(eShaderType::Default);
			shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 0.35f);
			level->renderPlayerPlannedBuilding(*shaderHandler, map);
			level->renderPlannedBuildings(*shaderHandler);
			shaderHandler->switchToShader(eShaderType::Debug);
			level->renderBasePositions(*shaderHandler);
			glEnable(GL_CULL_FACE);

			shaderHandler->switchToShader(eShaderType::Widjet);
			level->renderEntityStatusBars(*shaderHandler, camera, windowSize);
			level->renderEntitySelector(window, *shaderHandler);
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		ImGui_SFML_OpenGL3::endFrame();
		window.display();
	}

	level.reset();
	ImGui_SFML_OpenGL3::shutdown();

	return 0;
}