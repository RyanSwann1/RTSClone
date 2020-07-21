#include "Mesh.h"
#include "Globals.h"
#include "glad.h"

Mesh::Mesh()
	: m_vaoID(Globals::INVALID_OPENGL_ID),
	m_vboID(Globals::INVALID_OPENGL_ID),
	m_indicesID(Globals::INVALID_OPENGL_ID),
	m_vertices(),
	m_indices()
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
	glGenBuffers(1, &m_indicesID);
}

Mesh::~Mesh()
{
	assert(m_vaoID != Globals::INVALID_OPENGL_ID);
	glDeleteVertexArrays(1, &m_vaoID);

	assert(m_vboID != Globals::INVALID_OPENGL_ID);
	glDeleteBuffers(1, &m_vboID);

	assert(m_indicesID != Globals::INVALID_OPENGL_ID);
	glDeleteBuffers(1, &m_indicesID);
}

void Mesh::bind() const
{
    glBindVertexArray(m_vaoID);
}

void Mesh::attachToVAO()
{
    bind();
	assert(!m_vertices.empty());
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textCoords));
  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
}

void Mesh::render() const
{
    bind();
	assert(!m_indices.empty());
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}
