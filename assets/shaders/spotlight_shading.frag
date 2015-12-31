#version 330

// Input
in vec2 uvCoord;

// Output
out vec4 fragmentColor;

// Uniforms
uniform sampler2D uDiffuseTexture;
uniform sampler2D uPositionTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uMaterialTexture;
uniform sampler2DShadow uShadowMap;
uniform sampler2D uDirectionalLightingTexture;

uniform mat4 uViewMatrix;

uniform mat4 uLightMatrix;
uniform vec3 uLightPos;
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
uniform float uLightRange;

uniform float uLightShaftExposure = 0.4;
uniform float uLightShaftRange;
uniform int uLightShaftSamples;

float sampleShadowMap(vec3 vsSamplePos)
{
	float shadow = 0.0;
	vec4 smCoord = uLightMatrix * vec4(vsSamplePos, 1.0);
	if (smCoord.z > 0.0) shadow = textureProj(uShadowMap, smCoord);
	return shadow;
}

float calcLightScale(vec3 samplePos, vec3 vsLightPos)
{
	float lightDist = length(vsLightPos - samplePos);
	//return max((-1.0/uLightRange)*lightDist + 1.0, 0); // Linear
	return max((-1.0/(uLightRange*uLightRange))*(lightDist*lightDist-uLightRange*uLightRange), 0); // Quadratic
}

float lightShaftFactor(vec3 vsPos, int numSamples, float maxSampleDist, vec3 vsLightPos)
{
	vec3 camDir = normalize(vsPos);
	float sampleLength = min(length(vsPos), maxSampleDist) / float(numSamples+1);
	vec3 toNextSamplePos = camDir * sampleLength;

	vec3 currentSamplePos = toNextSamplePos;
	float factor = 0.0;
	for (int i = 0; i < numSamples; i++) {
		factor += sampleShadowMap(currentSamplePos) * calcLightScale(currentSamplePos, vsLightPos);
		currentSamplePos += toNextSamplePos;
	}
	factor /= float(numSamples);
	
	return factor;
}

void main()
{
	// Constants (that should probably be externally defined)
	const float materialShininess = 6;

	// Values from textures
	vec3 diffuseColor = texture(uDiffuseTexture, uvCoord).rgb;
	vec3 vsPos = texture(uPositionTexture, uvCoord).xyz;
	vec3 vsNormal = normalize(texture(uNormalTexture, uvCoord).xyz);
	vec3 material = texture(uMaterialTexture, uvCoord).xyz;
	float materialDiffuse = material.y;
	float materialSpecular = material.z;
	float shadow = sampleShadowMap(vsPos);
	vec3 dirLights = texture(uDirectionalLightingTexture, uvCoord).rgb;

	// Light calculation positions
	vec3 vsLightPos = (uViewMatrix * vec4(uLightPos, 1)).xyz;
	vec3 toLight = normalize(vsLightPos - vsPos);
	vec3 toCam = normalize(-vsPos);
	vec3 halfVec = normalize(toLight + toCam);

	// Light scaling
	float lightScale = calcLightScale(vsPos, vsLightPos);

	// Calculates diffuse and specular light
	float diffuseLightIntensity = max(dot(vsNormal, toLight), 0.0);
	float specularLightIntensity = 0.0;
	if (diffuseLightIntensity > 0.0) {
		float specularAngle = max(dot(vsNormal, halfVec), 0.0);
		specularLightIntensity = pow(specularAngle, materialShininess);
		//specularLightIntensity *= ((materialShininess + 2.0) / 8.0); // Normalization
		// Fresnel effect
		float fresnelBase = max(1.0 - max(dot(vsNormal, toCam), 0.0), 0.0);
		float fresnel = pow(fresnelBase, 5.0);
		materialSpecular = materialSpecular + (1.0-materialSpecular) * fresnel;
	}
	vec3 diffuseLight = uLightColor * diffuseLightIntensity * shadow * lightScale;
	vec3 specularLight = uLightColor * specularLightIntensity * shadow * lightScale;

	float lightShafts = lightShaftFactor(vsPos, uLightShaftSamples, uLightShaftRange, vsLightPos);

	vec3 shading = dirLights
	             + materialDiffuse * diffuseLight * diffuseColor
	             + materialSpecular * specularLight
	             + uLightShaftExposure * lightShafts * uLightColor;

	fragmentColor = vec4(shading, 1.0);
}