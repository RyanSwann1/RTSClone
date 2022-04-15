#include "OpenGLResource.h"
#include "Globals.h"
#include "glad.h"

OpenGLResourceBuffer::OpenGLResourceBuffer(GLenum target)
	: target(target)
{
	glGenBuffers(1, &id);
}

OpenGLResourceBuffer::OpenGLResourceBuffer(OpenGLResourceBuffer&& rhs) noexcept
	: target(rhs.target)
{
	std::swap(id, rhs.id);
}

OpenGLResourceBuffer& OpenGLResourceBuffer::operator=(OpenGLResourceBuffer&& rhs) noexcept
{
	std::swap(target, rhs.target);
	std::swap(id, rhs.id);
	return *this;
}

OpenGLResourceBuffer::~OpenGLResourceBuffer()
{
	if (id != 0)
	{
		glDeleteBuffers(1, &id);
	}
}

void OpenGLResourceBuffer::bind() const
{
	glBindBuffer(target, id);
}

unsigned int OpenGLResourceBuffer::getID() const
{
	return id;
}

OpenGLResourceVertexArray::OpenGLResourceVertexArray()
{
	glGenVertexArrays(1, &id);
}

OpenGLResourceVertexArray::OpenGLResourceVertexArray(OpenGLResourceVertexArray&& rhs) noexcept
{
	std::swap(id, rhs.id);
}

OpenGLResourceVertexArray& OpenGLResourceVertexArray::operator=(OpenGLResourceVertexArray&& rhs) noexcept
{
	std::swap(id, rhs.id);
	return *this;
}

OpenGLResourceVertexArray::~OpenGLResourceVertexArray()
{
	if (id != 0)
	{
		glDeleteVertexArrays(1, &id);
	}
}

unsigned int OpenGLResourceVertexArray::getID() const
{
	return id;
}

void OpenGLResourceVertexArray::bind() const
{
	glBindVertexArray(id);
}