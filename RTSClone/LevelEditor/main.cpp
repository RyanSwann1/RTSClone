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
#include <SFML/Graphics.hpp>
#include <iostream>

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

	SelectionBox selectionBox;
	GameObjectManager gameObjectManager = GameObjectManager::create("Level.txt");
	sf::Clock gameClock;
	Camera camera;
	glm::vec3 previousMousePosition = { 0.0f, Globals::GROUND_HEIGHT, 0.0f };

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
			switch(currentSFMLEvent.type)
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
				case sf::Keyboard::M:
				{
					glm::vec3 position = Globals::convertToNodePosition(camera.getMouseToGroundPosition(window));
					if (position != previousMousePosition)
					{
						previousMousePosition = position;
						gameObjectManager.addGameObject(eModelName::Meteor, previousMousePosition);
					}
				}
					break;
				case sf::Keyboard::Enter:
					LevelFileHandler::saveLevelToFile(gameObjectManager);
					break;
				case sf::Keyboard::R:
					gameObjectManager.removeGameObject(camera.getMouseToGroundPosition(window));
					break;
				case sf::Keyboard::O:
					glm::vec3 position = Globals::convertToNodePosition(camera.getMouseToGroundPosition(window));
					if (position != previousMousePosition)
					{
						previousMousePosition = position;
						gameObjectManager.addGameObject(eModelName::SatelliteDishLarge, previousMousePosition);
					}
					break;
				}
				break;
			case sf::Event::MouseButtonPressed:
				switch (currentSFMLEvent.mouseButton.button)
				{
				case sf::Mouse::Button::Left:
				{
					selectionBox.setStartingPosition(window, camera.getMouseToGroundPosition(window));
				}
					break;
				}
				break;
			case sf::Event::MouseButtonReleased:
				selectionBox.reset();
				break;
			case sf::Event::MouseMoved:
				if (selectionBox.active)
				{
					selectionBox.setSize(camera.getMouseToGroundPosition(window));
				}
				break;
			}
		}

		//Update
		camera.update(window, deltaTime);

		//Render
		glm::mat4 view = camera.getView();
		glm::mat4 projection = camera.getProjection(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderHandler->switchToShader(eShaderType::Default);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);
		shaderHandler->setUniform1f(eShaderType::Default, "uOpacity", 1.0f);

		gameObjectManager.render(*shaderHandler);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		shaderHandler->switchToShader(eShaderType::Debug);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Debug, "uProjection", projection);

#ifdef RENDER_AABB
		gameObjectManager.renderGameObjectAABB(*shaderHandler);
#endif // RENDER_AABB

		shaderHandler->switchToShader(eShaderType::SelectionBox);
		selectionBox.render(window);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		window.display();
	}

	return 0;
}