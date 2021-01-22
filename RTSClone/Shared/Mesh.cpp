#include "Mesh.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"

namespace
{
	const float HIGHLIGHTED_MESH_AMPLIFIER = 1.75f;
}

Mesh::Mesh()
	: vaoID(Globals::INVALID_OPENGL_ID),
	vboID(Globals::INVALID_OPENGL_ID),
	indiciesID(Globals::INVALID_OPENGL_ID),
	vertices(),
	indices(),
	material()
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &indiciesID);
}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, const Material& material)
	: vaoID(Globals::INVALID_OPENGL_ID),
	vboID(Globals::INVALID_OPENGL_ID),
	indiciesID(Globals::INVALID_OPENGL_ID),
	vertices(std::move(vertices)),
	indices(std::move(indices)),
	material(material)
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &indiciesID);
}

Mesh::Mesh(Mesh&& rhs) noexcept
	: vaoID(rhs.vaoID),
	vboID(rhs.vboID),
	indiciesID(rhs.indiciesID),
	vertices(std::move(rhs.vertices)),
	indices(std::move(rhs.indices)),
	material(rhs.material)
{
	rhs.vaoID = Globals::INVALID_OPENGL_ID;
	rhs.vboID = Globals::INVALID_OPENGL_ID;
	rhs.indiciesID = Globals::INVALID_OPENGL_ID;
}

Mesh& Mesh::operator=(Mesh&& rhs) noexcept
{
#ifdef GAME
	assert(vaoID != Globals::INVALID_OPENGL_ID &&
		vboID != Globals::INVALID_OPENGL_ID &&
		indiciesID != Globals::INVALID_OPENGL_ID);
#endif // GAME

	onDestroy();

	vaoID = rhs.vaoID;
	vboID = rhs.vboID;
	indiciesID = rhs.indiciesID;
	vertices = std::move(rhs.vertices);
	indices = std::move(rhs.indices);
	material = rhs.material;

	rhs.vaoID = Globals::INVALID_OPENGL_ID;
	rhs.vboID = Globals::INVALID_OPENGL_ID;
	rhs.indiciesID = Globals::INVALID_OPENGL_ID;

	return *this;
}

Mesh::~Mesh()
{
	onDestroy();
}

void Mesh::bind() const
{
    glBindVertexArray(vaoID);
}

void Mesh::attachToVAO() const
{
    bind();

	assert(!vertices.empty());
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));

	assert(!indices.empty());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiciesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

#if defined RENDER_AABB || defined RENDER_PATHING
void Mesh::renderDebugMesh(ShaderHandler& shaderHandler) const
{
	bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}
#endif // RENDER_AABB || defined RENDER_PATHING

void Mesh::render(ShaderHandler& shaderHandler, bool highlight) const
{
	assert(!indices.empty());
	bind();
	shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	if (highlight)
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", HIGHLIGHTED_MESH_AMPLIFIER);
	}
	else
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
	}

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool highlight) const
{
	assert(!indices.empty());
	bind();

	if (material.name == Globals::FACTION_MATERIAL_NAME_ID)
	{
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", 
			Globals::FACTION_COLORS[static_cast<int>(owningFactionController)]);
	}
	else
	{
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	}
	
	if (highlight)
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", HIGHLIGHTED_MESH_AMPLIFIER);
	}
	else
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
	}

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::onDestroy()
{
	if (vaoID != Globals::INVALID_OPENGL_ID &&
		vboID != Globals::INVALID_OPENGL_ID &&
		indiciesID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &vaoID);
		glDeleteBuffers(1, &vboID);
		glDeleteBuffers(1, &indiciesID);
	}
}

Vertex::Vertex(const glm::vec3& position)
	: position(position),
	normal()
{}

Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal)
	: position(position),
	normal(normal)
{}

//Material
Material::Material()
	: diffuse(),
	name()
{}

Material::Material(const glm::vec3 & diffuse, const std::string & name)
	: diffuse(diffuse),
	name(name)
{}