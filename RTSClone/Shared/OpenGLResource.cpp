#include "OpenGLResource.h"
#include "Globals.h"
#include "glad.h"

OpenGLResourceBuffer::OpenGLResourceBuffer(GLenum target)
	: ID(Globals::INVALID_OPENGL_ID),
	target(target)
{
	glGenBuffers(1, &ID);
}

OpenGLResourceBuffer::OpenGLResourceBuffer(OpenGLResourceBuffer&& rhs) noexcept
	: ID(rhs.ID),
	target(rhs.target)
{
	rhs.ID = Globals::INVALID_OPENGL_ID;
}

OpenGLResourceBuffer& OpenGLResourceBuffer::operator=(OpenGLResourceBuffer&& rhs) noexcept
{
	onDestroy();
	ID = rhs.ID;
	target = rhs.target;
	rhs.ID = Globals::INVALID_OPENGL_ID;

	return *this;
}

OpenGLResourceBuffer::~OpenGLResourceBuffer()
{
	onDestroy();
}

void OpenGLResourceBuffer::bind() const
{
	glBindBuffer(target, ID);
}

unsigned int OpenGLResourceBuffer::getID() const
{
	return ID;
}

void OpenGLResourceBuffer::onDestroy()
{
	if (ID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteBuffers(1, &ID);
	}
}

OpenGLResourceVertexArray::OpenGLResourceVertexArray()
	: ID(Globals::INVALID_OPENGL_ID)
{
	glGenVertexArrays(1, &ID);
}

OpenGLResourceVertexArray::OpenGLResourceVertexArray(OpenGLResourceVertexArray&& rhs) noexcept
	: ID(rhs.ID)
{
	rhs.ID = Globals::INVALID_OPENGL_ID;
}

OpenGLResourceVertexArray& OpenGLResourceVertexArray::operator=(OpenGLResourceVertexArray&& rhs) noexcept
{
	onDestroy();
	ID = rhs.ID;
	rhs.ID = Globals::INVALID_OPENGL_ID;

	return *this;
}

OpenGLResourceVertexArray::~OpenGLResourceVertexArray()
{
	onDestroy();
}

unsigned int OpenGLResourceVertexArray::getID() const
{
	return ID;
}

void OpenGLResourceVertexArray::bind() const
{
	glBindVertexArray(ID);
}

void OpenGLResourceVertexArray::onDestroy()
{
	if (ID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &ID);
	}
}