#pragma once

#include "glad/glad.h"

//glGenBuffers
struct OpenGLResourceBuffer
{
	OpenGLResourceBuffer(GLenum target);
	OpenGLResourceBuffer(const OpenGLResourceBuffer&) = delete;
	OpenGLResourceBuffer& operator=(const OpenGLResourceBuffer&) = delete;
	OpenGLResourceBuffer(OpenGLResourceBuffer&&) noexcept;
	OpenGLResourceBuffer& operator=(OpenGLResourceBuffer&&) noexcept;
	~OpenGLResourceBuffer();

	unsigned int getID() const;
	void bind() const;

private:
	unsigned int id = { 0 };
	GLenum target;
};

//glGenVertexArrays
struct OpenGLResourceVertexArray
{
	OpenGLResourceVertexArray();
	OpenGLResourceVertexArray(const OpenGLResourceVertexArray&) = delete;
	OpenGLResourceVertexArray& operator=(const OpenGLResourceVertexArray&) = delete;
	OpenGLResourceVertexArray(OpenGLResourceVertexArray&&) noexcept;
	OpenGLResourceVertexArray& operator=(OpenGLResourceVertexArray&&) noexcept;
	~OpenGLResourceVertexArray();

	unsigned int getID() const;
	void bind() const;

private:
	unsigned int id = { 0 };
};