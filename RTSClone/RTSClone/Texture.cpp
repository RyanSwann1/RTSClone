#include "Texture.h"
#include "glad.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Globals.h"
#include <array>

namespace
{
	const std::string TEXTURE_DIRECTORY = "Textures/";

	bool addTexture(const std::string& textureName)
	{
		sf::Image image;
		bool textureLoaded = image.loadFromFile(textureName);
		assert(textureLoaded);
		if (!textureLoaded)
		{
			std::cout << textureName << " not loaded.\n";
			return false;
		}
		image.flipVertically();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		glGenerateMipmap(GL_TEXTURE_2D);

		return true;
	}
}

//Texture
Texture::Texture(unsigned int ID)
	: m_ID(ID)
{}

Texture::~Texture()
{
	assert(m_ID != Globals::INVALID_OPENGL_ID);
	glDeleteTextures(1, &m_ID);
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

std::unique_ptr<Texture> Texture::create(const std::string& textureName)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	if (!addTexture(textureName))
	{
		return std::unique_ptr<Texture>();
	}
	else
	{
		return std::unique_ptr<Texture>(new Texture(textureID));
	}
}