#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "glad.h"
#include <SFML/Graphics.hpp>
#include "Model.h"
#include "ModelLoader.h"
#include "ShaderHandler.h"
//#define TINYOBJLOADER_IMPLEMENTATION
//#include "tiny_obj_loader.h"
#include "Texture.h"
#include "Camera.h"

//OpenGL Debug
//https://gist.github.com/qookei/76586d33238f0fa918c499dc7fb5ed04

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
	window.setMouseCursorVisible(false);
	window.setMouseCursorGrabbed(true);
	gladLoadGL();

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	glm::vec3 startingPosition = { 0.0f, 0.0f, 0.0f };
	int modelCount = 50;
	Camera camera;
	Model backpackModel;
	if (!ModelLoader::loadModel("models/backpack.obj", backpackModel))
	{
		std::cout << "Failed to load model: " << "backpack.obj\n";
		return -1;
	}

	backpackModel.attachMeshesToVAO();

	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";
	std::cout << glGetError() << "\n";

	while (window.isOpen())
	{
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
		}

		camera.update(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(camera.FOV),
			static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y), camera.nearPlaneDistance, camera.farPlaneDistance);

		shaderHandler->switchToShader(eShaderType::Default);
		
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);

		float zOffset = 10.0f;
		for (int i = 0; i < modelCount; ++i)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), startingPosition);
			shaderHandler->setUniformMat4f(eShaderType::Default, "uModel", model);


			backpackModel.render(*shaderHandler);
			startingPosition.z += zOffset;
		}
		
		startingPosition = glm::vec3();

		window.display();
	}


	return 0;
}