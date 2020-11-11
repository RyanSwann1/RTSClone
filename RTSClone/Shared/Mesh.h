#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>

//https://vulkan-tutorial.com/Loading_models

struct Material 
{
	Material();
	Material(const glm::vec3& diffuse, const std::string& name);

	glm::vec3 diffuse;
	std::string name;
};

struct MeshTextureDetails
{
	MeshTextureDetails(unsigned int ID, const std::string& type, const std::string& path);

	unsigned int ID;
	std::string type;
	std::string path; 
};

struct Vertex
{
	Vertex(const glm::vec3& position);
	Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textCoords);

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textCoords;
};

enum class eFactionController;
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

#ifdef LEVEL_EDITOR
	void renderDebugMesh(ShaderHandler& shaderHandler) const;
#endif // LEVEL_EDITOR
	void render(ShaderHandler& shaderHandler, bool selected = false) const;
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool selected = false) const;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<MeshTextureDetails> textures;
	Material material;

private:
	unsigned int vaoID;
	unsigned int vboID;
	unsigned int indiciesID;
};