#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"

struct Camera;
class ShaderHandler;
class Sprite : private NonCopyable
{
public:
	Sprite();
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void render(const glm::vec3& position, glm::uvec2 windowSize, float width, float height, float yOffset,
		ShaderHandler& shaderHandler, const Camera& camera, const glm::vec3& materialColor) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;

	void onDestroy();
};