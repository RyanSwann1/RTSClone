#version 330 core

layout(location = 0) in vec3 aPos; 
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 textCoords;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTextCoords;
out vec3 vNormal;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	vTextCoords = textCoords;
	
	vNormal = mat3(transpose(inverse(uModel))) * normal;
}