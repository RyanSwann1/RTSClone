#version 330 core

out vec4 color;
uniform sampler2D uTexture;

in vec2 vTextCoord;

void main()
{
	color = vec4(texture(uTexture, vTextCoord).rgb,1.0);
	//color = vec4(vTextCoord.x, 0, vTextCoord.y, 1.0);
	//color = texture(uTexture, vTextCoord);
	//color.g = 0;
	//color.b = 0;
};