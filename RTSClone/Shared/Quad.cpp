#include "Quad.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"
#include <array>

namespace
{
	const size_t QUAD_VERTEX_COUNT = 6;
	std::array<glm::vec3, QUAD_VERTEX_COUNT> getQuad(const glm::vec3& position, const glm::vec3& size)
	{
		return
		{
			position,
			{position.x + size.x, position.y, position.z},
			{position.x + size.x, position.y, position.z + size.z},
			{position.x + size.x, position.y, position.z + size.z},
			{position.x, position.y, position.z + size.z},
			position
		};
	}
}

Quad::Quad()
	: m_vboID(Globals::INVALID_OPENGL_ID),
	m_vaoID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
}

Quad::Quad(Quad&& rhs) noexcept
	: m_vaoID(rhs.m_vaoID),
	m_vboID(rhs.m_vboID)
{
	rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
	rhs.m_vboID = Globals::INVALID_OPENGL_ID;
}

Quad& Quad::operator=(Quad&& rhs) noexcept
{
	onDestroy();

	m_vaoID = rhs.m_vaoID;
	m_vboID = rhs.m_vboID;

	rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
	rhs.m_vboID = Globals::INVALID_OPENGL_ID;

	return *this;
}

Quad::~Quad()
{
	onDestroy();
}

void Quad::render(ShaderHandler& shaderHandler, const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float opacity) const
{
	glBindVertexArray(m_vaoID);
	std::array<glm::vec3, 6> quad = getQuad(position, size);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec3), quad.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, glm::vec3::length(), GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<const void*>(0));

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", color);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", opacity);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(QUAD_VERTEX_COUNT));
}

void Quad::onDestroy()
{
	assert((m_vaoID != Globals::INVALID_OPENGL_ID && m_vboID != Globals::INVALID_OPENGL_ID) || 
		m_vaoID == Globals::INVALID_OPENGL_ID && m_vboID == Globals::INVALID_OPENGL_ID);
	
	if (m_vaoID != Globals::INVALID_OPENGL_ID && m_vboID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &m_vaoID);
		glDeleteBuffers(1, &m_vboID);
	}
}