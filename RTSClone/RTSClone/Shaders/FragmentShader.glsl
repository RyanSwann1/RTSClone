#version 330 core

out vec4 color;
uniform sampler2DArray uTexture;

in vec3 vTextCoord;

void main()
{
	color = texture(uTexture, vTextCoord);
};