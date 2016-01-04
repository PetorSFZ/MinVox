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
uniform sampler2DShadow uShadowMap;
uniform Spotlight uSpotlight;

// Main
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void main()
{
	const int NUM_SAMPLES = 128;
	const float MAX_DIST = 40.0;

	float linDepth = texture(uLinearDepthTexture, uvCoord).r;
	vec3 vsPos = uFarPlaneDist * linDepth * nonNormRayDir / abs(nonNormRayDir.z);
	vec3 camDir = normalize(vsPos);
	vec3 sampleStep = (min(length(vsPos), MAX_DIST) / float(NUM_SAMPLES - 1)) * camDir;

	float factor = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		vec3 samplePos = float(i) * sampleStep;
		float sample = textureProj(uShadowMap, uSpotlight.lightMatrix * vec4(samplePos, 1.0));

		// Scale
		vec3 toSample = samplePos - uSpotlight.vsPos;
		vec3 toSampleDir = normalize(toSample);
		float lightDist = length(toSample);
		float rangeSquared = uSpotlight.range * uSpotlight.range;
		float lightDistSquared = lightDist * lightDist;
		float attenuation = smoothstep(uSpotlight.softAngleCos, uSpotlight.sharpAngleCos, dot(toSampleDir, uSpotlight.vsDir));
		float scale = attenuation * max((-1.0 / rangeSquared) * (lightDistSquared - rangeSquared), 0);

		factor += sample * scale;
	}
	factor /= float(NUM_SAMPLES);

	outFragColor = vec4(factor * uSpotlight.color, 1.0);
}