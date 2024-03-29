#version 330 core

out vec4 color;

uniform vec3 uMaterialColour;
uniform vec3 uAdditionalColour = vec3(1.0);
uniform float uSelectedAmplifier;
uniform float uOpacity;
in vec3 vNormal;

const float ambientFactor = 0.4;

void main()
{
	float dotFactor = dot(vNormal, vec3(0.0, 1.0, 0.0)) * 0.5 + 0.5;
	float darkenFactor = ambientFactor + dotFactor * (1.0 - ambientFactor);
	vec3 outputColour = uMaterialColour * darkenFactor;

	color = vec4(outputColour * uAdditionalColour * uSelectedAmplifier, uOpacity);
};