#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"

class ShaderHandler;
class PlayableAreaDisplay : private NonMovable, private NonCopyable
{
public:
	PlayableAreaDisplay();
	~PlayableAreaDisplay();

	void setSize(const glm::ivec2& size);
	void render(ShaderHandler& shaderHandler) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;
};