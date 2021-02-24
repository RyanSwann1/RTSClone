#pragma once

#include "glm/glm.hpp"

struct Camera;
class ShaderHandler;
class Sprite 
{
public:
	Sprite();
	Sprite(const Sprite&) = delete;
	Sprite& operator=(const Sprite&) = delete;
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void render(const glm::vec3& position, glm::uvec2 windowSize, float originalWidth, float spriteWidth, float height, float yOffset,
		ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;

	void onDestroy();
};