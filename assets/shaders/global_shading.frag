#version 330

precision highp float; // required by GLSL spec Sect 4.5.3

// Input
in vec2 uvCoord;

// Output
out vec4 fragmentColor;

// Uniforms
uniform sampler2D uDiffuseTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uMaterialTexture;
uniform sampler2D uAOTexture;
uniform sampler2D uDirectionalLightsTexture;

uniform vec3 uAmbientLight = vec3(0.25);

void main()
{
	// Values from textures
	vec3 diffuseColor = texture(uDiffuseTexture, uvCoord).rgb;
	vec3 emissive = texture(uEmissiveTexture, uvCoord).rgb;
	vec3 material = texture(uMaterialTexture, uvCoord).xyz;
	float materialAmbient = material.x;
	float ao = texture(uAOTexture, uvCoord).r;
	vec3 dirLights = texture(uDirectionalLightsTexture, uvCoord).rgb;

	vec3 shading = emissive
	             + materialAmbient * uAmbientLight * diffuseColor * ao
	             + dirLights;

	fragmentColor = vec4(shading, 1.0);
}