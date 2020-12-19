#version 330 core

out vec4 color;

uniform vec3 uMaterialColor;

void main()
{
	color = vec4(uMaterialColor, 1.0);
};