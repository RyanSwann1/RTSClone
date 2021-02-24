#pragma once

//glGenBuffers
struct OpenGLResourceBuffer
{
	OpenGLResourceBuffer();
	OpenGLResourceBuffer(const OpenGLResourceBuffer&) = delete;
	OpenGLResourceBuffer& operator=(const OpenGLResourceBuffer&) = delete;
	OpenGLResourceBuffer(OpenGLResourceBuffer&&) noexcept;
	OpenGLResourceBuffer& operator=(OpenGLResourceBuffer&&) noexcept;
	~OpenGLResourceBuffer();

	unsigned int getID() const;

private:
	unsigned int ID;

	void onDestroy();
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

private:
	unsigned int ID;

	void onDestroy();
};