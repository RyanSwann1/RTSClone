#pragma once

#include "glm/glm.hpp"
#include "OpenGLResource.h"
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
struct Mesh
{
	Mesh();
	Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, const Material& material);
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = default;
	Mesh& operator=(Mesh&&) = default;

	void attachToVAO() const;

#if defined RENDER_AABB || defined RENDER_PATHING
	void renderDebugMesh(ShaderHandler& shaderHandler) const;
#endif // defined RENDER_AABB || defined RENDER_PATHING

	void render(ShaderHandler& shaderHandler, const glm::vec3& additionalColor, float opacity) const;
	void render(ShaderHandler& shaderHandler, bool highlight = false) const;
	void render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool highlight = false) const;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Material material;

private:
	OpenGLResourceVertexArray vaoID;
	OpenGLResourceBuffer vboID;
	OpenGLResourceBuffer indiciesID;
};