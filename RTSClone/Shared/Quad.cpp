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

Quad::Quad(const glm::vec3& size, const glm::vec3& color, float opacity)
	: m_opacity(opacity),
	m_position(0.0f),
	m_size(size),
	m_color(color),
	m_AABB(m_position, m_size),
	m_VAO(),
	m_VBO(GL_ARRAY_BUFFER)
{}

Quad::Quad(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float opacity)
	: m_opacity(opacity),
	m_position(position),
	m_size(size),
	m_color(color),
	m_AABB(position, m_size),
	m_VAO(),
	m_VBO(GL_ARRAY_BUFFER)
{}

const glm::vec3& Quad::getPosition() const
{
	return m_position;
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
	m_AABB.update(m_position, m_size);
}

void Quad::render(ShaderHandler& shaderHandler) const
{
	m_VBO.bind();
	std::array<glm::vec3, QUAD_VERTEX_COUNT> quad = getQuad(m_position, m_size);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec3), quad.data(), GL_STATIC_DRAW);

	m_VAO.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, glm::vec3::length(), GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<const void*>(0));

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", m_color);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", m_opacity);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(QUAD_VERTEX_COUNT));
}

void Quad::render(ShaderHandler& shaderHandler, const glm::vec3& color) const
{
	m_VBO.bind();
	std::array<glm::vec3, QUAD_VERTEX_COUNT> quad = getQuad(m_position, m_size);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec3), quad.data(), GL_STATIC_DRAW);

	m_VAO.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, glm::vec3::length(), GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<const void*>(0));

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", color);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", m_opacity);

	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(QUAD_VERTEX_COUNT));
}