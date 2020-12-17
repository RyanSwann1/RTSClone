#pragma once

#include "NonCopyable.h"
#include "glm/glm.hpp"

class HealthBarSprite : private NonCopyable
{
public:
	HealthBarSprite();
	HealthBarSprite(HealthBarSprite&&) noexcept;
	HealthBarSprite& operator=(HealthBarSprite&&) noexcept;
	~HealthBarSprite();

	void render(glm::vec2 position, glm::uvec2 windowSize, float width, float yOffset) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;

	void onDestroy();
};