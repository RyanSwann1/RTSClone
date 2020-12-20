#include "Sprite.h"
#include "glad.h"
#include "Globals.h"
#include "Entity.h"
#include "Camera.h"
#include "ShaderHandler.h"
#include <array>

namespace
{
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

Sprite::Sprite()
	: m_vaoID(Globals::INVALID_OPENGL_ID),
	m_vboID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &m_vaoID);
	glGenBuffers(1, &m_vboID);
}

Sprite::Sprite(Sprite&& rhs) noexcept
	: m_vaoID(rhs.m_vaoID),
	m_vboID(rhs.m_vboID)
{
	rhs.m_vaoID = Globals::INVALID_OPENGL_ID;
	rhs.m_vboID = Globals::INVALID_OPENGL_ID;
}

Sprite& Sprite::operator=(Sprite&& rhs) noexcept
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

Sprite::~Sprite()
{
	onDestroy();
}

void Sprite::render(const glm::vec3& position, glm::uvec2 windowSize, float width, float height, float yOffset, 
	ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor) const
{
	glm::vec4 positionNDC = camera.getProjection(glm::ivec2(windowSize.x, windowSize.y)) * camera.getView() * glm::vec4(position, 1.0f);
	positionNDC /= positionNDC.w;

	std::array<glm::vec2, QUAD_VERTEX_COUNT> quad = getQuadCoords(
		glm::vec2(positionNDC),
		width / windowSize.x * 2.0f,
		height / windowSize.y * 2.0f,
		yOffset / windowSize.y * 2.0f);

	shaderHandler.setUniformVec3(eShaderType::HealthBar, "uMaterialColor", materialColor);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec2), quad.data(), GL_STATIC_DRAW);

	glBindVertexArray(m_vaoID);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);

	glDrawArrays(GL_TRIANGLES, 0, QUAD_VERTEX_COUNT);

	//float width = 0.0f;
	//float height = DEFAULT_HEALTH_BAR_HEIGHT / windowSize.y * 2.0f;
	//float yOffset = 0.0f;
	//float currentHealth = static_cast<float>(m_health) / static_cast<float>(m_maximumHealth);
	//switch (getEntityType())
	//{
	//case eEntityType::Unit:
	//	width = Globals::UNIT_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = UNIT_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::Worker:
	//	width = Globals::WORKER_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = WORKER_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::HQ:
	//	width = Globals::HQ_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = HQ_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::SupplyDepot:
	//	width = Globals::SUPPLY_DEPOT_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = SUPPLY_DEPOT_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::Barracks:
	//	width = Globals::BARRACKS_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = BARRACKS_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::Turret:
	//	width = Globals::TURRET_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = TURRET_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//case eEntityType::Laboratory:
	//	width = Globals::LABORATORY_STAT_BAR_WIDTH * currentHealth / windowSize.x * 2.0f;
	//	yOffset = LABORATORY_HEALTH_BAR_YOFFSET / windowSize.y * 2.0f;
	//	break;
	//default:
	//	assert(false);
	//}
}

//void Sprite::render(glm::vec2 position, glm::uvec2 windowSize, float width, float height, float yOffset) const
//{ 
//	std::array<glm::vec2, QUAD_VERTEX_COUNT> quad = getQuadCoords(
//		position, 
//		width, 
//		height,
//		yOffset);
//
//	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
//	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec2), quad.data(), GL_STATIC_DRAW);
//
//	glBindVertexArray(m_vaoID);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);
//
//	glDrawArrays(GL_TRIANGLES, 0, QUAD_VERTEX_COUNT);
//}

void Sprite::onDestroy()
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