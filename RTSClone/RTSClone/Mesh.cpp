#include "Mesh.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<TextureDetails>&& textures)
	: m_vaoID(Globals::INVALID_OPENGL_ID),
	m_vboID(Globals::INVALID_OPENGL_ID),
	m_indicesID(Globals::INVALID_OPENGL_ID),
	m_vertices(std::move(vertices)),
	m_indices(std::move(indices)),
	m_textures(std::move(textures))
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
	m_textures(std::move(orig.m_textures))
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, textCoords));
  
	assert(!m_indices.empty());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
}

void Mesh::render(ShaderHandler& shaderHandler) const
{
	assert(m_textures.size() == static_cast<size_t>(1));
	glBindTexture(GL_TEXTURE_2D, m_textures.front().id);

    bind();
	assert(!m_indices.empty());
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}