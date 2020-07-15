#version 330 core

out vec4 color;
uniform sampler2DArray uTexture;

in vec3 vTextCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
	color = texture(uTexture, vTextCoord);
};