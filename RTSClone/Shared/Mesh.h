#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>

//https://vulkan-tutorial.com/Loading_models

struct Material 
{
	glm::vec3 Diffuse;
};

struct MeshTextureDetails
{
	MeshTextureDetails(unsigned int ID, const std::string& type, const std::string& path);

	unsigned int ID;
	std::string type;
	std::string path;  // we store the path of the texture to compare with other textures
};

struct Vertex
{
	Vertex(const glm::vec3& position);
	Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textCoords);

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textCoords;
};

class ShaderHandler;
struct Mesh : private NonCopyable
{
	Mesh();
	Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<MeshTextureDetails>&& textures, const Material& material);
	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;
	~Mesh();

	void bind() const;
	void attachToVAO() const;
	void render(ShaderHandler& shaderHandler, bool selected = false) const;

	unsigned int vaoID;
	unsigned int vboID;
	unsigned int indiciesID;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<MeshTextureDetails> textures;
	Material material;
};