#version 330 core

out vec4 color;
uniform sampler2D uTexture;

in vec2 vTextCoord;

void main()
{
	color = texture(uTexture, vTextCoord);
};