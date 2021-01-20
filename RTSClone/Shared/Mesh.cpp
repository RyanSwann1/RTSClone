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

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, textCoords));

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

void Mesh::render(ShaderHandler& shaderHandler, bool highlighted) const
{
	assert(!indices.empty());
	bind();
	shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	if (highlighted)
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", HIGHLIGHTED_MESH_AMPLIFIER);
	}
	else
	{
		shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
	}

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
}

void Mesh::render(ShaderHandler& shaderHandler, eFactionController owningFactionController, bool highlighted) const
{
	assert(!indices.empty());
	bind();

	if (material.name == Globals::FACTION_MATERIAL_NAME_ID)
	{
		glm::vec3 diffuseMaterial = glm::vec3(1.0f);
		switch (owningFactionController)
		{
		case eFactionController::Player:
			diffuseMaterial = Globals::PLAYER_MATERIAL_DIFFUSE;
			break;
		case eFactionController::AI_1:
			diffuseMaterial = Globals::AI_1_MATERIAL_DIFFUSE;
			break;
		case eFactionController::AI_2:
			diffuseMaterial = Globals::AI_2_MATERIAL_DIFFUSE;
			break;
		case eFactionController::AI_3:
			diffuseMaterial = Globals::AI_3_MATERIAL_DIFFUSE;
			break;
		default:
			assert(false);
		}

		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", diffuseMaterial);
	}
	else
	{
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.diffuse);
	}
	
	if (highlighted)
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
	normal(),
	textCoords()
{}

Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& textCoords)
	: position(position),
	normal(normal),
	textCoords(textCoords)
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