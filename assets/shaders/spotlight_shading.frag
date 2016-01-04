#version 330

// Structs
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct Spotlight {
	vec3 vsPos;
	vec3 vsDir;
	vec3 color;
	float range;
	float softFovRad; // outer
	float sharpFovRad; //inner
	float softAngleCos; // outer
	float sharpAngleCos; // inner
	mat4 lightMatrix;
};

// Input, output and uniforms
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

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
uniform sampler2DShadow uShadowMap;

uniform Spotlight uSpotlight;

// Helper functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

float sampleShadowMap(vec3 vsSamplePos)
{
	return textureProj(uShadowMap, uSpotlight.lightMatrix * vec4(vsSamplePos, 1.0));
}

float calcLightScale(vec3 samplePos)
{
	vec3 toSample = samplePos - uSpotlight.vsPos;
	vec3 toSampleDir = normalize(toSample);
	float rangeSquared = uSpotlight.range * uSpotlight.range;
	float lightDistSquared = dot(toSample, toSample);
	float attenuation = smoothstep(uSpotlight.softAngleCos, uSpotlight.sharpAngleCos, dot(toSampleDir, uSpotlight.vsDir));
	return attenuation * max((-1.0 / rangeSquared) * (lightDistSquared - rangeSquared), 0);
}

// Helper functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void main()
{
	// Values from GBuffer
	float linDepth = texture(uLinearDepthTexture, uvCoord).r;
	vec3 vsPos = uFarPlaneDist * linDepth * nonNormRayDir / abs(nonNormRayDir.z);
	vec3 vsNormal = texture(uNormalTexture, uvCoord).xyz;
	vec3 diffuseColor = texture(uDiffuseTexture, uvCoord).rgb;
	vec3 material = texture(uMaterialTexture, uvCoord).rgb;

	float mtlAmbient = material.r;
	float mtlDiffuse = material.g;
	float mtlSpecular = material.b;
	const float mtlShininess = 6;

	// Vectors
	vec3 toCam = normalize(-vsPos);
	vec3 toLight = normalize(uSpotlight.vsPos - vsPos);
	vec3 halfVec = normalize(toLight + toCam);

	// Diffuse lighting
	float diffuseIntensity = clamp(dot(toLight, vsNormal), 0.0, 1.0);
	vec3 diffuseContribution = diffuseIntensity * diffuseColor * mtlDiffuse * uSpotlight.color;

	// Fresnel effect
	vec3 materialSpecular = vec3(mtlSpecular);
	float fresnelBase = clamp(1.0 - clamp(dot(vsNormal, toCam), 0.0, 1.0), 0.0, 1.0);
	float fresnel = pow(fresnelBase, 5.0);
	materialSpecular = materialSpecular + (vec3(1.0) - materialSpecular) * fresnel;

	// Specular lighting
	float specularAngle = 0.0;
	if (diffuseIntensity > 0.0) {
		specularAngle = clamp(dot(vsNormal, halfVec), 0.0, 1.0);
	}
	float specularIntensity = pow(specularAngle, mtlShininess);
	specularIntensity *= ((mtlShininess + 2.0) / 8.0); // Normalization
	vec3 specularContribution = specularIntensity * materialSpecular * uSpotlight.color;

	// Shadow & lightscale
	float lightScale = calcLightScale(vsPos);
	float shadow = sampleShadowMap(vsPos);

	// Total shading and output
	vec3 shading = diffuseContribution * shadow * lightScale
	             + specularContribution * shadow * lightScale;

	outFragColor = vec4(shading, 1.0);
}