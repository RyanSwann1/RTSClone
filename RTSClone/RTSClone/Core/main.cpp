#include <iostream>

#include "glad/glad.h"
#include <SFML/Graphics.hpp>
#include "Graphics/ModelLoader.h"
#include "Graphics/ShaderHandler.h"
#include "Core/Map.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Factions/FactionPlayer.h"
#include "Factions/FactionAI.h"
#include "Graphics/ModelManager.h"
#include "Core/Mineral.h"
#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>
#include "Core/Level.h"
#include "UI/UIManager.h"
#include "Events/GameMessenger.h"
#include "Core/PathFinding.h"
#include "Core/LevelFileHandler.h"

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

	PathFinding::getInstance();

	sf::Clock gameClock;
	UIManager uiManager;
	const std::array<std::string, Globals::MAX_LEVELS> levelNames = LevelFileHandler::loadLevelNames();
	std::optional<Level> currentLevel = {};

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
					(currentLevel ? currentLevel.reset() : window.close());
				}
				break;
			}

			if (currentLevel)
			{
				currentLevel->handleInput(windowSize, window, currentSFMLEvent, uiManager);
			}
		}

		ImGui_SFML_OpenGL3::startFrame();
		//ImGui::ShowDemoWindow();

		if (currentLevel)
		{
			const Faction* winningFaction = currentLevel->getWinningFaction();
			if (winningFaction)
			{
				broadcast<GameMessages::UIDisplayWinner>({ winningFaction->getController() });
				currentLevel.reset();
			}
		}
		else
		{
			ImGui::Begin("Level Selection");
			for (const auto& levelName : levelNames)
			{
				if (!levelName.empty() && ImGui::Button(levelName.c_str()))
				{
					broadcast<GameMessages::UIClearWinner>({});
					if (std::optional<LevelDetailsFromFile> levelDetails = Level::load(levelName, windowSize))
					{
						currentLevel.emplace(std::move(*levelDetails), windowSize);
					}

					if (!currentLevel)
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
		if (currentLevel)
		{	
			currentLevel->update(deltaTime, uiManager, windowSize, window);
		}

		//Render
		if (currentLevel)
		{
			glm::mat4 view = currentLevel->getCamera().getView();
			glm::mat4 projection = currentLevel->getCamera().getProjection(glm::ivec2(window.getSize().x, window.getSize().y));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shaderHandler->switchToShader(eShaderType::Default);
			shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
			shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);
			shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 1.0f);

			currentLevel->render(*shaderHandler);
		
			glDisable(GL_CULL_FACE);
			shaderHandler->switchToShader(eShaderType::Debug);
			shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
			shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

			currentLevel->renderTerrain(*shaderHandler);
			
			glEnable(GL_CULL_FACE);
			glEnable(GL_BACK);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);
#ifdef RENDER_AABB
			shaderHandler->switchToShader(eShaderType::Debug);
			shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
			shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);
			
			level->renderAABB(*shaderHandler);
#endif // RENDER_AABB
#ifdef RENDER_PATHING
			currentLevel->renderPathing(*shaderHandler);
#endif // RENDER_PATHING
			glDisable(GL_CULL_FACE);
			shaderHandler->switchToShader(eShaderType::Default);
			shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 0.35f);
			currentLevel->renderPlayerPlannedBuilding(*shaderHandler);
			currentLevel->renderPlannedBuildings(*shaderHandler);
			shaderHandler->switchToShader(eShaderType::Debug);
			currentLevel->renderBasePositions(*shaderHandler);

			shaderHandler->switchToShader(eShaderType::Widjet);
			currentLevel->renderEntityStatusBars(*shaderHandler, windowSize);
			currentLevel->renderEntitySelector(window, *shaderHandler);
			currentLevel->renderMinimap(*shaderHandler, windowSize, window);
			glEnable(GL_CULL_FACE);
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		ImGui_SFML_OpenGL3::endFrame();
		window.display();
	}

	currentLevel.reset();
	ImGui_SFML_OpenGL3::shutdown();

	return 0;
}