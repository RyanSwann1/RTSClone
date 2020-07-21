#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <string>
#include <memory>
#include <vector>

class Texture : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<Texture> create(const std::string& textureName);
	~Texture();

	void bind() const;
	void unbind() const;

private:
	Texture(unsigned int ID);
	const unsigned int m_ID;
};
