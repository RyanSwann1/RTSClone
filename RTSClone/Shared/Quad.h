#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"

class ShaderHandler;
class Quad : private NonCopyable
{
public:
	Quad();
	Quad(Quad&&) noexcept;
	Quad& operator=(Quad&&) noexcept;
	~Quad();

	void render(ShaderHandler& shaderHandler, const glm::vec3& position, const glm::vec3& size,
		const glm::vec3& color, float opacity = 1.0f) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;

	void onDestroy();
};