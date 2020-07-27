#version 330 core

out vec4 color;

uniform vec3 uColor;

void main()
{
	color = vec4(uColor, 1.0); 
};