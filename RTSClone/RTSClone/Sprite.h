#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"

class Sprite : private NonCopyable
{
public:
	Sprite();
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void render(glm::vec2 position, glm::uvec2 windowSize, float width, float height, float yOffset) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;

	void onDestroy();
};