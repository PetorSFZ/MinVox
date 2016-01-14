#version 330

// Input
in vec2 uvCoord;
in vec3 nonNormRayDir;

// Output
out vec4 outFragColor;

// Uniforms
uniform float uFarPlaneDist;
uniform sampler2D uLinearDepthTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uDiffuseTexture;
uniform sampler2D uMaterialTexture;
uniform sampler2D uAOTexture;
uniform sampler2D uSpotlightTexture;
uniform sampler2D uLightShaftsTexture;

uniform vec3 uAmbientLight;
uniform int uOutputSelect = 1;

void main()
{
	float linDepth = texture(uLinearDepthTexture, uvCoord).r;
	vec3 vsPos = uFarPlaneDist * linDepth * nonNormRayDir / abs(nonNormRayDir.z);
	vec3 vsNormal = texture(uNormalTexture, uvCoord).rgb;
	vec3 diffuse = texture(uDiffuseTexture, uvCoord).rgb;
	vec3 material = texture(uMaterialTexture, uvCoord).rgb;
	float ao = texture(uAOTexture, uvCoord).r;
	vec3 spotlightShading = texture(uSpotlightTexture, uvCoord).rgb;
	vec3 lightShafts = texture(uLightShaftsTexture, uvCoord).rgb;

	vec3 shading = material.x * uAmbientLight * diffuse * ao
	             + spotlightShading
	             + lightShafts;

	switch (uOutputSelect) {
	case 1: outFragColor = vec4(shading, 1); break;
	
	case 2: outFragColor = vec4(vsPos, 1); break;
	case 3: outFragColor = vec4(vsNormal, 1); break;
	case 4: outFragColor = vec4(diffuse, 1); break;
	case 5: outFragColor = vec4(material, 1); break;
	
	case 6: outFragColor = vec4(vec3(ao), 1); break;
	case 7: outFragColor = vec4(spotlightShading, 1); break;
	case 8: outFragColor = vec4(lightShafts, 1); break;
	}
}