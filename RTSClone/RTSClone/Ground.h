#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"

class Ground : private NonMovable, private NonCopyable
{
public:
	Ground();
	~Ground();

	void render() const;

private:
	unsigned int m_vaoID;
	unsigned int m_vboID;
};