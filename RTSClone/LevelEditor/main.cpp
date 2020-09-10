#include "glm/glm.hpp"
#include "glad.h"
#include "ModelManager.h"
#include "ShaderHandler.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"
#include "Globals.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include "PlayableAreaDisplay.h"
#include <SFML/Graphics.hpp>
#include <iostream>

#include <imgui/imgui.h>
#include <imgui_impl/imgui_wrapper.h>

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

	const std::string levelName = "Level.txt";
	PlayableAreaDisplay playableAreaDisplay;
	SelectionBox selectionBox;
	GameObjectManager gameObjectManager = GameObjectManager::create(levelName);
	sf::Clock gameClock;
	Camera camera;
	glm::vec3 previousMousePosition = { 0.0f, Globals::GROUND_HEIGHT, 0.0f };
	PlannedGameObject plannedGameObject;
	plannedGameObject.modelName = eModelName::RocksTall;
	plannedGameObject.active = true;
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
			switch (currentSFMLEvent.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				switch (currentSFMLEvent.key.code)
				{
				case sf::Keyboard::Escape:
					window.close();
					break;
				break;
				case sf::Keyboard::Enter:
					LevelFileHandler::saveLevelToFile(levelName, gameObjectManager);
					break;
				case sf::Keyboard::R:
					gameObjectManager.removeGameObject(camera.getMouseToGroundPosition(window));
					break;
				case sf::Keyboard::T:
					gameObjectManager.removeAllSelectedGameObjects();
					break;
				}
			case sf::Event::MouseButtonPressed:
			{
				switch (currentSFMLEvent.mouseButton.button)
				{
				case sf::Mouse::Button::Left:
				{
					glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
					selectionBox.setStartingPosition(window, mouseToGroundPosition);
					gameObjectManager.addGameObject(plannedGameObject.modelName, plannedGameObject.position);
				}
				break;
				}
			}
				break;
			case sf::Event::MouseButtonReleased:
				selectionBox.reset();
				break;
			case sf::Event::MouseMoved:
			{
				glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
				if (selectionBox.active)
				{
					selectionBox.setSize(mouseToGroundPosition);
					gameObjectManager.update(selectionBox);
				}
				
				glm::vec3 newPosition = Globals::convertToNodePosition(mouseToGroundPosition);
				AABB AABB(newPosition, ModelManager::getInstance().getModel(plannedGameObject.modelName));
				if (Globals::isWithinMapBounds(AABB))
				{
					plannedGameObject.position = newPosition;
				}
			}
				break;
			}
		}

		//Update
		camera.update(deltaTime);
		ImGui_SFML_OpenGL3::startFrame();

		ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Models", nullptr, ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Close"))
					{
						window.close();
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::BeginChild("left pane", ImVec2(175, 0), true);
			const auto& modelNames = ModelManager::getInstance().getModelNames();
			for (int i = 0; i < modelNames.size(); i++)
			{
				std::string label = "Model: " + modelNames[i];
				if (ImGui::Selectable(label.c_str(), selected == i))
				{
					selected = i;
					plannedGameObject.modelName = ModelManager::getInstance().getModelName(modelNames[i]);
				}
			}
			ImGui::EndChild();
			ImGui::SameLine();
		}
		ImGui::End();

		// DEMO!
		//ImGui::ShowDemoWindow(); //No idea if that is the correct name btw

		//Render
		glm::mat4 view = camera.getView();
		glm::mat4 projection = camera.getProjection(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);

		shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 1.0f);
		gameObjectManager.render(*shaderHandler);
		plannedGameObject.render(*shaderHandler);

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