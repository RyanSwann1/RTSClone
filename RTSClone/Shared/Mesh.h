#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <vector>
#include <string>

struct Material 
{
	Material();
	Material(const glm::vec3& diffuse, const std::string& name);

	glm::vec3 diffuse;
	std::string name;
};

struct Vertex
{
	Vertex(const glm::vec3& position);
	Vertex(const glm::vec3& position, const glm::vec3& normal);

	glm::vec3 position;
	glm::vec3 normal;
};

enum class eFactionController;
class ShaderHandler;
struct Mesh : private NonCopyable
{
	Mesh();
	Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, const Material& material);
	Mesh(Mesh&&) noexcept;
	Mesh& operator=(Mesh&&) noexcept;
	~Mesh();

	void bind() const;
	void attachToVAO() const;

#if defined RENDER_AABB || defined RENDER_PATHING
	void renderDebugMesh(ShaderHandler& shaderHandler) const;
#endif // defined RENDER_AABB || defined RENDER_PATHING

	void render(ShaderHandler& shaderHandler, bool highlight = false) const;
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool highlight = false) const;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material material;

private:
	unsigned int vaoID;
	unsigned int vboID;
	unsigned int indiciesID;

	void onDestroy();
};