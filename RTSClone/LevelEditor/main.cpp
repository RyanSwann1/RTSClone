#include "glm/glm.hpp"
#include "glad.h"
#include "ModelManager.h"
#include "ShaderHandler.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"
#include "Globals.h"
#include "EntityManager.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include "PlayableAreaDisplay.h"
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

constexpr size_t MAX_LEVELS = 5;
bool isLevelNameAvailable(const std::array<std::string, MAX_LEVELS>& levelNames, std::string& availableLevelName)
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

	const std::array<std::string, MAX_LEVELS> levelNames =
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

	PlayableAreaDisplay playableAreaDisplay;
	std::unique_ptr<Level> level;
	SelectionBox selectionBox;
	sf::Clock gameClock;
	Camera camera;
	bool plannedEntityActive = false;
	bool showGUIWindow = false;
	eWindowState currentWindowState = eWindowState::None;
	Entity plannedEntity(eModelName::RocksTall, { 0.0f, 0.0f, 0.0f });
	int selected = 0;	

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
				level->handleInput(currentSFMLEvent, selectionBox, camera, plannedEntityActive, window, plannedEntity);
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
				break;
			case sf::Event::MouseButtonPressed:
				if (currentSFMLEvent.mouseButton.button == sf::Mouse::Button::Left)
				{
					if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
					{
						glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
						if (level)
						{
							bool entitySelected = level->getEntityManager().isEntitySelected();
							if (plannedEntityActive && !entitySelected)
							{
								level->addEntity(plannedEntity.getModelName(), plannedEntity.getPosition());
							}
							else
							{
								plannedEntityActive = false;
								selectionBox.setStartingPosition(window, mouseToGroundPosition);
							}
						}
					}
				}
				else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Button::Right)
				{
					plannedEntityActive = false;
				}
				break;
			case sf::Event::MouseButtonReleased:
				selectionBox.reset();
				break;
			case sf::Event::MouseWheelMoved:
				camera.zoom(currentSFMLEvent.mouseWheel.delta);
				break;
			case sf::Event::MouseMoved:
			{
				if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_::ImGuiHoveredFlags_AnyWindow))
				{
					glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
					if (selectionBox.isActive())
					{
						selectionBox.setSize(mouseToGroundPosition);
					}

					if (level)
					{
						glm::vec3 newPosition = Globals::convertToNodePosition(mouseToGroundPosition);
						AABB AABB(newPosition, ModelManager::getInstance().getModel(plannedEntity.getModelName()));
						if (Globals::isWithinMapBounds(AABB, level->getMapSize()))
						{
							plannedEntity.setPosition(newPosition);
						}
					}
				}
			}
				break;
			}
		}

		//Update
		camera.update(deltaTime);
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
				ImGui::BeginChild("left pane", ImVec2(175, 250), true);
				const auto& modelNames = ModelManager::getInstance().getModelNames();
				for (int i = 0; i < modelNames.size(); i++)
				{
					if (ImGui::Selectable(modelNames[i].c_str(), selected == i))
					{
						selected = i;
						plannedEntity.setModelName(ModelManager::getInstance().getModelName(modelNames[i]));
						plannedEntityActive = true;
					}
				}
				ImGui::EndChild();
				ImGui::SameLine();
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
					level->handlePlayerDetails(showGUIWindow);
				}	
				break;
			case eWindowState::LevelDetails:
				if (level)
				{
					level->handleLevelDetails(showGUIWindow, playableAreaDisplay);
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
								playableAreaDisplay.setSize({ 0, 0 });
								level.reset();	
							}

							level = Level::load(levelName);
							assert(level);
							playableAreaDisplay.setSize(level->getMapSize());
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
						playableAreaDisplay.setSize({ 0, 0 });
						level.reset();
					}
			
					level = Level::create(availableLevelName);
					assert(level);
					playableAreaDisplay.setSize(level->getMapSize());
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
								playableAreaDisplay.setSize({ 0, 0 });
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

		if (plannedEntityActive)
		{
			plannedEntity.render(*shaderHandler);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

		playableAreaDisplay.render(*shaderHandler);

#ifdef RENDER_AABB
		gameObjectManager.renderGameObjectAABB(*shaderHandler);
#endif // RENDER_AABB

		shaderHandler->switchToShader(eShaderType::SelectionBox);
		selectionBox.render(window);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		ImGui_SFML_OpenGL3::endFrame(); 
		window.display();
	}

	return 0;
}