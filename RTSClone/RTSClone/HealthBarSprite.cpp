#include "HealthBarSprite.h"
#include "glad.h"
#include "Globals.h"
#include "Entity.h"
#include <array>

namespace
{
	const int DEFAULT_WIDTH = 50;
	const int DEFAULT_HEIGHT = 10;
	const int QUAD_VERTEX_COUNT = 6;

	std::array<glm::vec2, QUAD_VERTEX_COUNT> getQuadCoords(glm::vec2 position, float width, float height, float yOffset)
	{
		return
		{
			glm::vec2(position.x, position.y + yOffset),
			glm::vec2(position.x + width, position.y + yOffset),
			glm::vec2(position.x + width, position.y + height + yOffset),
			glm::vec2(position.x + width, position.y + height + yOffset),
			glm::vec2(position.x, position.y + height + yOffset),
			glm::vec2(position.x, position.y + yOffset)
		};
	};
}

HealthBarSprite::HealthBarSprite()
	: m_vaoID(Globals::INVALID_OPENGL_ID),
	m_vboID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
}

HealthBarSprite::HealthBarSprite(HealthBarSprite&& rhs) noexcept
	: m_vaoID(rhs.m_vaoID),
	m_vboID(rhs.m_vboID)
{
	rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
	rhs.m_vboID = Globals::INVALID_OPENGL_ID;
}

HealthBarSprite& HealthBarSprite::operator=(HealthBarSprite&& rhs) noexcept
{
	assert(this != &rhs);
	if (this != &rhs)
	{
		onDestroy();

		m_vaoID = rhs.m_vaoID;
		m_vboID = rhs.m_vboID;

		rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
		rhs.m_vboID = Globals::INVALID_OPENGL_ID;
	}

	return *this;
}

HealthBarSprite::~HealthBarSprite()
{
	onDestroy();
}

void HealthBarSprite::render(glm::vec2 position, glm::uvec2 windowSize, float width, float yOffset) const
{ 
	std::array<glm::vec2, QUAD_VERTEX_COUNT> quad = getQuadCoords(
		position, 
		width, 
		static_cast<float>(DEFAULT_HEIGHT) / windowSize.y * 2.0f,
		yOffset);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec2), quad.data(), GL_STATIC_DRAW);

	glBindVertexArray(m_vaoID);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);

	glDrawArrays(GL_TRIANGLES, 0, QUAD_VERTEX_COUNT);
}

void HealthBarSprite::onDestroy()
{
	if (m_vaoID != Globals::INVALID_OPENGL_ID && m_vboID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &m_vaoID);
		glDeleteBuffers(1, &m_vboID);
	}
	else
	{
		assert(m_vaoID == Globals::INVALID_OPENGL_ID && m_vboID == Globals::INVALID_OPENGL_ID);
	}
}