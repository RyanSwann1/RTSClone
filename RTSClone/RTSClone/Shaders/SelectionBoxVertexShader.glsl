#version 330 core

layout(location = 0) in vec2 aPos; 

uniform mat4 uOrthographic;

void main()
{
	gl_Position = uOrthographic * vec4(aPos, 0.0, 1.0);
}