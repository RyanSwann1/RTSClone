#include "OpenGLResource.h"
#include "Globals.h"
#include "glad.h"

OpenGLResourceBuffer::OpenGLResourceBuffer()
	: ID(Globals::INVALID_OPENGL_ID)
{}

OpenGLResourceBuffer::OpenGLResourceBuffer(OpenGLResourceBuffer&& rhs) noexcept
	: ID(rhs.ID)
{
	rhs.ID = Globals::INVALID_ENTITY_ID;
}

OpenGLResourceBuffer& OpenGLResourceBuffer::operator=(OpenGLResourceBuffer&& rhs) noexcept
{
	onDestroy();
	ID = rhs.ID;
	rhs.ID = Globals::INVALID_OPENGL_ID;

	return *this;
}

OpenGLResourceBuffer::~OpenGLResourceBuffer()
{
	onDestroy();
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
{}

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

void OpenGLResourceVertexArray::onDestroy()
{
	if (ID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &ID);
	}
}