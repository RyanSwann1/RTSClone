#version 330 core

out vec4 color;
uniform sampler2D uTexture;

in vec2 vTextCoord;

void main()
{
	color = texture(uTexture, vTextCoord);
};
	//color = vec4(texture(uTexture, vTextCoord).rgb,1.0);
	//color = vec4(0.1, 0.1, 0.1, 1.0);
	//color = vec4(vTextCoord.x, 0, vTextCoord.y, 1.0);

	//color.g = 0;
	//color.b = 0;
//};