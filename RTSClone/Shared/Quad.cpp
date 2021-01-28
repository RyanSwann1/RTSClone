#include "Quad.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"
#include <array>

namespace
{
	const float DEFAULT_OPACITY = 1.0f;
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

Quad::Quad(const glm::vec3& size, const glm::vec3& color, float opacity)
	: m_opacity(opacity),
	m_position(),
	m_size(size),
	m_color(color),
	m_AABB(m_position, m_size),
	m_vboID(Globals::INVALID_OPENGL_ID),
	m_vaoID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
}

Quad::Quad(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float opacity)
	: m_opacity(opacity),
	m_position(position),
	m_size(size),
	m_color(color),
	m_AABB(position, m_size),
	m_vboID(Globals::INVALID_OPENGL_ID),
	m_vaoID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
}

Quad::Quad(Quad&& rhs) noexcept
	: m_opacity(rhs.m_opacity),
	m_position(rhs.m_position),
	m_size(rhs.m_size),
	m_color(rhs.m_color),
	m_AABB(std::move(rhs.m_AABB)),
	m_vaoID(rhs.m_vaoID),
	m_vboID(rhs.m_vboID)
{
	rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
	rhs.m_vboID = Globals::INVALID_OPENGL_ID;
}

Quad& Quad::operator=(Quad&& rhs) noexcept
{
	onDestroy();

	m_opacity = rhs.m_opacity;
	m_position = rhs.m_position;
	m_size = rhs.m_size;
	m_color = rhs.m_color;
	m_AABB = std::move(rhs.m_AABB);
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

const AABB& Quad::getAABB() const
{
	return m_AABB;
}

void Quad::setSize(const glm::vec3& size)
{
	m_size = size;
}

void Quad::setPosition(const glm::vec3& position)
{
	m_position = position;
	m_AABB.update(position, m_size);
}

void Quad::render(ShaderHandler& shaderHandler) const
{
	glBindVertexArray(m_vaoID);
	std::array<glm::vec3, QUAD_VERTEX_COUNT> quad = getQuad(m_position, m_size);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec3), quad.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, glm::vec3::length(), GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<const void*>(0));

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", m_color);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", m_opacity);

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