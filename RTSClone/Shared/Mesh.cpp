#include "Mesh.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"

namespace
{
	constexpr float SELECTED_MESH_AMPLIFIER = 1.75f;
}

Mesh::Mesh()
	: vaoID(Globals::INVALID_OPENGL_ID),
	vboID(Globals::INVALID_OPENGL_ID),
	indiciesID(Globals::INVALID_OPENGL_ID),
	vertices(),
	indices(),
	textures(),
	material()
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &indiciesID);
}

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<MeshTextureDetails>&& textures, const Material& material)
	: vaoID(Globals::INVALID_OPENGL_ID),
	vboID(Globals::INVALID_OPENGL_ID),
	indiciesID(Globals::INVALID_OPENGL_ID),
	vertices(std::move(vertices)),
	indices(std::move(indices)),
	textures(std::move(textures)),
	material(material)
{
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboID);
	glGenBuffers(1, &indiciesID);
}

Mesh::Mesh(Mesh&& orig) noexcept
	: vaoID(orig.vaoID),
	vboID(orig.vboID),
	indiciesID(orig.indiciesID),
	vertices(std::move(orig.vertices)),
	indices(std::move(orig.indices)),
	textures(std::move(orig.textures)),
	material(orig.material)
{
	orig.vaoID = Globals::INVALID_OPENGL_ID;
	orig.vboID = Globals::INVALID_OPENGL_ID;
	orig.indiciesID = Globals::INVALID_OPENGL_ID;
}

Mesh& Mesh::operator=(Mesh&& orig) noexcept
{
	vaoID = orig.vaoID;
	vboID = orig.vboID;
	indiciesID = orig.indiciesID;
	vertices = std::move(orig.vertices);
	indices = std::move(orig.indices);
	textures = std::move(orig.textures);
	material = orig.material;

	orig.vaoID = Globals::INVALID_OPENGL_ID;
	orig.vboID = Globals::INVALID_OPENGL_ID;
	orig.indiciesID = Globals::INVALID_OPENGL_ID;

	return *this;
}

Mesh::~Mesh()
{
	if (vaoID != Globals::INVALID_OPENGL_ID &&
		vboID != Globals::INVALID_OPENGL_ID &&
		indiciesID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &vaoID);
		glDeleteBuffers(1, &vboID);
		glDeleteBuffers(1, &indiciesID);
	}
	else
	{
		assert(vaoID == Globals::INVALID_OPENGL_ID &&
			vboID == Globals::INVALID_OPENGL_ID &&
			indiciesID == Globals::INVALID_OPENGL_ID);
	}
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

void Mesh::render(ShaderHandler& shaderHandler, bool selected) const
{
	if (!textures.empty())
	{
		assert(textures.size() == static_cast<size_t>(1));
		glBindTexture(GL_TEXTURE_2D, textures.front().ID);

		assert(!indices.empty());
		bind();
		switch (shaderHandler.getActiveShaderType())
		{
		case eShaderType::Default:
			shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", { 1.0f, 1.0f, 1.0f });
			break;
		default:
			assert(false);
		}
		
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		assert(!indices.empty());
		bind();
		switch (shaderHandler.getActiveShaderType())
		{
		case eShaderType::Default:
			shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", material.Diffuse);

			if (selected)
			{
				shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", SELECTED_MESH_AMPLIFIER);
			}
			else
			{
				shaderHandler.setUniform1f(eShaderType::Default, "uSelectedAmplifier", 1.0f);
			}
			break;
		}
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
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

MeshTextureDetails::MeshTextureDetails(unsigned int ID, const std::string& type, const std::string& path)
	: ID(ID),
	type(type),
	path(path)
{}