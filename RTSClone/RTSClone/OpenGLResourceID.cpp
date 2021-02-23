#include "OpenGLResourceID.h"
#include "Globals.h"
#include "glad.h"

OpenGLResourceBufferID::OpenGLResourceBufferID()
	: ID(Globals::INVALID_OPENGL_ID)
{}

OpenGLResourceBufferID::OpenGLResourceBufferID(OpenGLResourceBufferID&& rhs) noexcept
	: ID(rhs.ID)
{
	rhs.ID = Globals::INVALID_ENTITY_ID;
}

OpenGLResourceBufferID& OpenGLResourceBufferID::operator=(OpenGLResourceBufferID&& rhs) noexcept
{
	onDestroy();
	ID = rhs.ID;
	rhs.ID = Globals::INVALID_OPENGL_ID;

	return *this;
}

OpenGLResourceBufferID::~OpenGLResourceBufferID()
{
	onDestroy();
}

void OpenGLResourceBufferID::onDestroy()
{
	if (ID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteBuffers(1, &ID);
	}
}

OpenGLResourceVertexArrayID::OpenGLResourceVertexArrayID()
	: ID(Globals::INVALID_OPENGL_ID)
{}

OpenGLResourceVertexArrayID::OpenGLResourceVertexArrayID(OpenGLResourceVertexArrayID&& rhs) noexcept
	: ID(rhs.ID)
{
	rhs.ID = Globals::INVALID_OPENGL_ID;
}

OpenGLResourceVertexArrayID& OpenGLResourceVertexArrayID::operator=(OpenGLResourceVertexArrayID&& rhs) noexcept
{
	onDestroy();
	ID = rhs.ID;
	rhs.ID = Globals::INVALID_OPENGL_ID;

	return *this;
}

OpenGLResourceVertexArrayID::~OpenGLResourceVertexArrayID()
{
	onDestroy();
}

void OpenGLResourceVertexArrayID::onDestroy()
{
	if (ID != Globals::INVALID_OPENGL_ID)
	{
		glDeleteVertexArrays(1, &ID);
	}
}