#version 330 core

layout(location = 0) in vec3 aPos; 
layout(location = 1) in vec3 normal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	vNormal = mat3(transpose(inverse(uModel))) * normal;
}