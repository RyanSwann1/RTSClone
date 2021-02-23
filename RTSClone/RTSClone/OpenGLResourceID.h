#pragma once

//glGenBuffers
struct OpenGLResourceBufferID
{
	OpenGLResourceBufferID();
	OpenGLResourceBufferID(const OpenGLResourceBufferID&) = delete;
	OpenGLResourceBufferID& operator=(const OpenGLResourceBufferID&) = delete;
	OpenGLResourceBufferID(OpenGLResourceBufferID&&) noexcept;
	OpenGLResourceBufferID& operator=(OpenGLResourceBufferID&&) noexcept;
	~OpenGLResourceBufferID();

	unsigned int ID;

private:
	void onDestroy();
};

//glGenVertexArrays
struct OpenGLResourceVertexArrayID
{
	OpenGLResourceVertexArrayID();
	OpenGLResourceVertexArrayID(const OpenGLResourceVertexArrayID&) = delete;
	OpenGLResourceVertexArrayID& operator=(const OpenGLResourceVertexArrayID&) = delete;
	OpenGLResourceVertexArrayID(OpenGLResourceVertexArrayID&&) noexcept;
	OpenGLResourceVertexArrayID& operator=(OpenGLResourceVertexArrayID&&) noexcept;
	~OpenGLResourceVertexArrayID();

	unsigned int ID;

private:
	void onDestroy();
};