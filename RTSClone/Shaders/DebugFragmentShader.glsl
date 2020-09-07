#version 330 core

out vec4 color;

uniform vec3 uColor;
uniform float uOpacity;

void main()
{
	color = vec4(uColor, uOpacity); 
};