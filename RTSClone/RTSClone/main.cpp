#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "glad.h"
#include <SFML/Graphics.hpp>
#include "Mesh.h"
#include "ShaderHandler.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Texture.h"
#include "Camera.h"

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
	gladLoadGL();

	glViewport(0, 0, windowSize.x, windowSize.y);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	std::unique_ptr<ShaderHandler> shaderHandler = ShaderHandler::create();
	assert(shaderHandler);
	if (!shaderHandler)
	{
		std::cout << "Shader Handler not loaded\n";
		return -1;
	}

	std::unique_ptr<Texture> vikingRoomTexture = Texture::create("textures/viking_room.png");
	assert(vikingRoomTexture);
	if (!vikingRoomTexture)
	{
		std::cout << "viking_room.png not found\n";
		return -1;
	}

	vikingRoomTexture->bind();

	Camera camera;
	Mesh mesh;
	std::string modelName = { "models/viking_room.obj" };
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelName.c_str())) 
	{
		std::cout << "Couldn't load model\n";
		return -1;
	}

	//if (!warn.empty()) {
	//	std::cout << warn << std::endl;
	//}

	//if (!err.empty()) {
	//	std::cerr << err << std::endl;
	//}

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex;

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.textCoords = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			//vertex.textCoords = {
			//	attrib.texcoords[2 * index.texcoord_index + 0],
			//	attrib.texcoords[2 * index.texcoord_index + 1]
			//};

			mesh.m_vertices.push_back(vertex);
			mesh.m_indices.push_back(mesh.m_indices.size());
		}
	}

	mesh.attachToVAO();

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
		shaderHandler->setUniformMat4f(eShaderType::Default, "uModel", model);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uView", view);
		shaderHandler->setUniformMat4f(eShaderType::Default, "uProjection", projection);

		mesh.render();

		window.display();
	}


	return 0;
}