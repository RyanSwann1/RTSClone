#include "Sprite.h"
#include "glad.h"
#include "Globals.h"
#include "Entity.h"
#include "Camera.h"
#include "ShaderHandler.h"

namespace
{
	std::array<glm::ivec2, QUAD_VERTEX_COUNT> getQuadCoords(glm::ivec2 position, glm::ivec2 size)
	{
		return
		{
			position, 
			glm::ivec2(position.x + size.x, position.y),
			glm::ivec2(position.x + size.x, position.y + size.y),
			glm::ivec2(position.x + size.x, position.y + size.y),
			glm::ivec2(position.x, position.y + size.y),
			position
		};
	};

	std::array<glm::vec2, QUAD_VERTEX_COUNT> getQuadCoords(glm::vec2 position, float width, float height, float yOffset = 0.0f)
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
	: m_VAO(),
	m_VBO(GL_ARRAY_BUFFER)
{}

void Sprite::render(const glm::vec3& position, glm::uvec2 windowSize, float originalWidth, float spriteWidth, float height, float yOffset, 
	ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor, float opacity) const
{
	glm::vec4 positionNDC = camera.getProjection(glm::ivec2(windowSize.x, windowSize.y)) * camera.getView() * glm::vec4(position, 1.0f);
	positionNDC /= positionNDC.w;

	float originalWidthNDC = (2.0f * originalWidth) / windowSize.x;
	std::array<glm::vec2, QUAD_VERTEX_COUNT> quad = getQuadCoords(
		{ positionNDC.x - originalWidthNDC / 2.0f, positionNDC.y },
		(2.0f * spriteWidth) / windowSize.x,
		(2.0f * height) / windowSize.y,
		(2.0f * yOffset) / windowSize.y);

	shaderHandler.setUniformVec3(eShaderType::Widjet, "uColor", materialColor);
	shaderHandler.setUniform1f(eShaderType::Widjet, "uOpacity", opacity);

	fillBuffer(quad);
}

void Sprite::render(glm::vec2 position, glm::vec2 size, const glm::vec3& color, ShaderHandler& shaderHandler,
	glm::uvec2 windowSize, float opacity) const
{
	shaderHandler.setUniformVec3(eShaderType::Widjet, "uColor", color);
	shaderHandler.setUniform1f(eShaderType::Widjet, "uOpacity", opacity);

	glm::vec2 positionNDC = {
		((position.x * 2.0f) / windowSize.x) - 1.0f,
		 ((position.y * 2.0f) / windowSize.y) - 1.0f
	};

	glm::vec2 sizeNDC = {
		(size.x * 2.0f) / windowSize.x,
		(size.y * 2.0f) / windowSize.y
	};

	fillBuffer(getQuadCoords(positionNDC, sizeNDC.x, sizeNDC.y));
}

void Sprite::fillBuffer(const std::array<glm::vec2, QUAD_VERTEX_COUNT>& quad) const
{
	m_VBO.bind();
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(glm::vec2), quad.data(), GL_STATIC_DRAW);

	m_VAO.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);

	glDrawArrays(GL_TRIANGLES, 0, QUAD_VERTEX_COUNT);
}
