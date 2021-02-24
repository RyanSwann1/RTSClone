#pragma once

#include "OpenGLResource.h"
#include "glm/glm.hpp"

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
		ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor) const;

private:
	OpenGLResourceVertexArray m_VAO;
	OpenGLResourceBuffer m_VBO;
};