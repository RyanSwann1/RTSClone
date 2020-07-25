#include "Mesh.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<MeshTextureDetails>&& textures, const Material& material)
	: m_vaoID(Globals::INVALID_OPENGL_ID),
	m_vboID(Globals::INVALID_OPENGL_ID),
	m_indicesID(Globals::INVALID_OPENGL_ID),
	m_vertices(std::move(vertices)),
	m_indices(std::move(indices)),
	m_textures(std::move(textures)),
	m_material(material)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
	glGenBuffers(1, &m_indicesID);
}

Mesh::Mesh(Mesh&& orig) noexcept
	: m_vaoID(orig.m_vaoID),
	m_vboID(orig.m_vboID),
	m_indicesID(orig.m_indicesID),
	m_vertices(std::move(orig.m_vertices)),
	m_indices(std::move(orig.m_indices)),
	m_textures(std::move(orig.m_textures)),
	m_material(orig.m_material)
{
	orig.m_vaoID = Globals::INVALID_OPENGL_ID;
	orig.m_vboID = Globals::INVALID_OPENGL_ID;
	orig.m_indicesID = Globals::INVALID_OPENGL_ID;
}

Mesh& Mesh::operator=(Mesh&& orig) noexcept
{
	m_vaoID = orig.m_vaoID;
	m_vboID = orig.m_vboID;
	m_indicesID = orig.m_indicesID;
	m_vertices = std::move(orig.m_vertices);
	m_indices = std::move(orig.m_indices);
	m_textures = std::move(orig.m_textures);
	m_material = orig.m_material;

	orig.m_vaoID = Globals::INVALID_OPENGL_ID;
	orig.m_vboID = Globals::INVALID_OPENGL_ID;
	orig.m_indicesID = Globals::INVALID_OPENGL_ID;

	return *this;
}

Mesh::~Mesh()
{
	if (m_vaoID != Globals::INVALID_OPENGL_ID &&
		m_vboID != Globals::INVALID_OPENGL_ID &&
		m_indicesID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &m_vaoID);
		glDeleteBuffers(1, &m_vboID);
		glDeleteBuffers(1, &m_indicesID);
	}
	else
	{
		assert(m_vaoID == Globals::INVALID_OPENGL_ID &&
			m_vboID == Globals::INVALID_OPENGL_ID &&
			m_indicesID == Globals::INVALID_OPENGL_ID);
	}
}

void Mesh::bind() const
{
    glBindVertexArray(m_vaoID);
}

void Mesh::attachToVAO() const
{
    bind();

	assert(!m_vertices.empty());
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, textCoords));


  
	assert(!m_indices.empty());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
}

void Mesh::render(ShaderHandler& shaderHandler) const
{
	if (!m_textures.empty())
	{
		assert(m_textures.size() == static_cast<size_t>(1));
		glBindTexture(GL_TEXTURE_2D, m_textures.front().ID);

		assert(!m_indices.empty());
		bind();
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", { 1.0f, 1.0f, 1.0f });
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		assert(!m_indices.empty());
		bind();
		shaderHandler.setUniformVec3(eShaderType::Default, "uMaterialColour", m_material.Diffuse);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	}
}

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