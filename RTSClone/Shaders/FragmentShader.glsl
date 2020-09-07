#version 330 core

out vec4 color;

uniform vec3 uMaterialColour;
uniform float uSelectedAmplifier;
uniform float uOpacity;
in vec2 vTextCoords;
in vec3 vNormal;

const float ambientFactor = 0.5;

void main()
{
	float dotFactor = dot(vNormal, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
	float darkenFactor = ambientFactor + dotFactor * (1.0 - ambientFactor);
	vec3 outputColour = uMaterialColour * darkenFactor;
	color = vec4(outputColour * uSelectedAmplifier, uOpacity);
};