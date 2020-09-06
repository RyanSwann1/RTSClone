#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"

class ShaderHandler;
class Ground : private NonMovable, private NonCopyable
{
public:
	Ground();
	~Ground();

	void render(ShaderHandler& shaderHandler) const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;
};