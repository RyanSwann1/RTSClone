#include "glm/glm.hpp"
#include "glad.h"
#include "ModelManager.h"
#include "ShaderHandler.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"
#include "Globals.h"
#include "EntityManager.h"
#include "LevelFileHandler.h"
#include "Player.h"
#include "Level.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>

//https://eliasdaler.github.io/using-imgui-with-sfml-pt2/#getting-back-to-the-context-of-the-window-tree-etc

enum class eWindowState
{
	None,
	PlayerDetails,
	LevelDetails,
	LoadLevel,
	CreateLevel,
	RemoveLevel
};

bool isLevelNameAvailable(const std::array<std::string, Globals::MAX_LEVELS>& levelNames, std::string& availableLevelName)
{
	for (const auto& levelName : levelNames)
	{
		if (!LevelFileHandler::isLevelExists(levelName))
		{
			availableLevelName = levelName;
			return true;
		}
	}

	return false;
}

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
	sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "Level Editor", sf::Style::Default, settings);
	//sf::Window window(sf::VideoMode(windowSize.x, windowSize.y), "Level Editor", sf::Style::Fullscreen, settings);
	window.setFramerateLimit(60);
	window.setMouseCursorGrabbed(true);
	gladLoadGL();

	ImGui_SFML_OpenGL3::init(window);

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);

	if (!ModelManager::getInstance().isAllModelsLoaded())
	{
		std::cout << "Failed to load all models\n";
		return -1;
	}

	const std::array<std::string, Globals::MAX_LEVELS> levelNames =
	{
		"Level1.txt",
		"Level2.txt",
		"Level3.txt",
		"Level4.txt",
		"Level5.txt"
	};

	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	shaderHandler->switchToShader(eShaderType::SelectionBox);
	shaderHandler->setUniformMat4f(eShaderType::SelectionBox, "uOrthographic", glm::ortho(0.0f, static_cast<float>(windowSize.x),
		static_cast<float>(windowSize.y), 0.0f));

	std::unique_ptr<Level> level;
	sf::Clock gameClock;
	Camera camera;
	bool showGUIWindow = false;
	eWindowState currentWindowState = eWindowState::None;
	glm::ivec2 lastMousePosition = { 0, 0 };
	glm::ivec2 lastMouseButtonPress = { 0, 0 };
	bool mouseMoved = false;

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
			if (level)
			{
				level->handleInput(currentSFMLEvent, camera, window);
			}

			switch (currentSFMLEvent.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (currentSFMLEvent.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}
				if (currentSFMLEvent.key.code == sf::Keyboard::LAlt)
				{
					camera.zoom(window, lastMousePosition);
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (currentSFMLEvent.mouseButton.button == sf::Mouse::Button::Right)
				{
					sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);
				}
				break;
			case sf::Event::MouseWheelMoved:
				camera.zoom(currentSFMLEvent.mouseWheel.delta);
				break;
			case sf::Event::MouseMoved:
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
				{
					mouseMoved = true;
				}
			}
				break;
			}
		}

		//Update
		if (mouseMoved)
		{
			mouseMoved = false;
			camera.onMouseMove(window, deltaTime, lastMousePosition);
			window.setMouseCursorVisible(false);
		}
		camera.update(deltaTime, window, lastMousePosition);
		lastMousePosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
		ImGui_SFML_OpenGL3::startFrame();
		ImGui::SetNextWindowSize(ImVec2(175, static_cast<float>(windowSize.y)), ImGuiCond_FirstUseEver);
		std::string titleName = "Level Editor";
		if (level)
		{
			titleName = level->getName();
		}
		if (ImGui::Begin(titleName.c_str(), nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (level && ImGui::MenuItem("Player Details"))
					{
						showGUIWindow = true;
						currentWindowState = eWindowState::PlayerDetails;
					}
					if (level && ImGui::MenuItem("Level Details"))
					{
						showGUIWindow = true;
						currentWindowState = eWindowState::LevelDetails;
					}
					if (ImGui::MenuItem("Create Level"))
					{
						showGUIWindow = true;
						currentWindowState = eWindowState::CreateLevel;
					}
					if (ImGui::MenuItem("Load Level"))
					{
						showGUIWindow = true;
						currentWindowState = eWindowState::LoadLevel;
					}
					if (ImGui::MenuItem("Remove Level"))
					{
						showGUIWindow = true;
						currentWindowState = eWindowState::RemoveLevel;
					}
					if (ImGui::MenuItem("Save Level"))
					{
						if (level)
						{
							level->save();
						}
					}
					if (ImGui::MenuItem("Close"))
					{
						window.close();
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			if (level)
			{
				level->handleModelNamesGUI();
			}
		}
		ImGui::End();

		if (showGUIWindow)
		{
			switch (currentWindowState)
			{
			case eWindowState::None:
				break;
			case eWindowState::PlayerDetails:
				if (level)
				{
					level->handlePlayerDetailsGUI(showGUIWindow);
				}	
				break;
			case eWindowState::LevelDetails:
				if (level)
				{
					level->handleLevelDetailsGUI(showGUIWindow);
				}
				break;
			case eWindowState::LoadLevel:
			{
				ImGui::Begin("Load Level", &showGUIWindow, ImGuiWindowFlags_None);
				for (const auto& levelName : levelNames)
				{
					if (LevelFileHandler::isLevelExists(levelName))
					{
						if (ImGui::Button(levelName.c_str()))
						{
							if (level)
							{
								LevelFileHandler::saveLevelToFile(*level);
								level.reset();	
							}

							level = Level::load(levelName);
						}
					}
				}
				ImGui::End();
			}
			break;
			case eWindowState::CreateLevel:
			{
				std::string availableLevelName;
				if (isLevelNameAvailable(levelNames, availableLevelName))
				{
					if (level)
					{
						LevelFileHandler::saveLevelToFile(*level);
						level.reset();
					}
			
					level = Level::create(availableLevelName);
				}

				currentWindowState = eWindowState::None;
			}
				break;
			case eWindowState::RemoveLevel:
				for (const auto& levelName : levelNames)
				{
					if (LevelFileHandler::isLevelExists(levelName))
					{
						if (ImGui::Button(levelName.c_str()))
						{
							if(level && level->getName() == levelName)
							{
								level.reset();
							}

							LevelFileHandler::removeLevel(levelName);
							break;
						}
					}
				}
				break;
			default:
				assert(false);
			}
		}

		//Demo
		ImGui::ShowDemoWindow();

		//Render
		glm::mat4 view = camera.getView();
		glm::mat4 projection = camera.getProjection(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);

		shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 1.0f);
		if (level)
		{
			level->render(*shaderHandler);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

		if (level)
		{
			level->renderPlayableArea(*shaderHandler);
		}

#ifdef RENDER_AABB
		if (level)
		{
			level->renderAABB(*shaderHandler);
		}
#endif // RENDER_AABB

		if (level)
		{
			shaderHandler->switchToShader(eShaderType::SelectionBox);
			level->renderSelectionBox(window);
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		ImGui_SFML_OpenGL3::endFrame(); 
		window.display();
	}

	return 0;
}