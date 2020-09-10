#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"

class ShaderHandler;
class PlayableAreaDisplay : private NonMovable, private NonCopyable
{
public:
	PlayableAreaDisplay();
	~PlayableAreaDisplay();

	void render(ShaderHandler& shaderHandler) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;
};