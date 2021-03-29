#pragma once

#include "OpenGLResource.h"
#include <array>
#include "glm/glm.hpp"

static const int QUAD_VERTEX_COUNT = 6;

struct Camera;
class ShaderHandler;
class Sprite 
{
public:
	Sprite();
	Sprite(const Sprite&) = delete;
	Sprite& operator=(const Sprite&) = delete;
	Sprite(Sprite&&) = default;
	Sprite& operator=(Sprite&&) = default;

	void render(const glm::vec3& position, glm::uvec2 windowSize, float originalWidth, float spriteWidth, float height, float yOffset,
		ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor, float opacity = 1.0f) const;
	
	void render(glm::vec2 position, glm::vec2 size, const glm::vec3& color, ShaderHandler& shaderHandler,
		glm::uvec2 windowSize, float opacity = 1.0f) const;

private:
	OpenGLResourceVertexArray m_VAO;
	OpenGLResourceBuffer m_VBO;

	void fillBuffer(const std::array<glm::vec2, QUAD_VERTEX_COUNT>& quad) const;
};