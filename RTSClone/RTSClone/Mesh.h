#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>

//https://vulkan-tutorial.com/Loading_models

struct MeshTextureDetails
{
	MeshTextureDetails()
		: id(),
		type(),
		path()
	{}

	unsigned int id;
	std::string type;
	std::string path;  // we store the path of the texture to compare with other textures
};

struct Vertex
{
	Vertex()
		: position(),
		textCoords()
	{}

	glm::vec3 position;
	glm::vec2 textCoords;
};

class ShaderHandler;
struct Mesh : private NonCopyable
{
	Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<MeshTextureDetails>&& textures);
	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;
	~Mesh();

	void bind() const;
	void attachToVAO() const;
	void render(ShaderHandler& shaderHandler) const;

	unsigned int m_vaoID;
	unsigned int m_vboID;
	unsigned int m_indicesID;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<MeshTextureDetails> m_textures;
};