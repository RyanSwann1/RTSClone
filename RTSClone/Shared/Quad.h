#pragma once

#include "AABB.h"
#include "glm/glm.hpp"
#include "OpenGLResource.h"

class ShaderHandler;
struct Quad
{
public:
	Quad(const glm::vec3& size, const glm::vec3& color, float opacity = 1.0f);
	Quad(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color, float opacity = 1.0f);
	Quad(const Quad&) = delete;
	Quad& operator=(const Quad&) = delete;
	Quad(Quad&&) = default;
	Quad& operator=(Quad&&) = default;

	const glm::vec3& getSize() const;
	const glm::vec3& getPosition() const;
	const AABB& getAABB() const;

	void setSize(const glm::vec3& size);
	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;
	void render(ShaderHandler& shaderHandler, const glm::vec3& color) const;

private:
	float m_opacity;
	glm::vec3 m_position;
	glm::vec3 m_size;
	glm::vec3 m_color;
	AABB m_AABB;
	OpenGLResourceVertexArray m_VAO;
	OpenGLResourceBuffer m_VBO;
};