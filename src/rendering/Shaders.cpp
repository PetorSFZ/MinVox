#include "rendering/Shaders.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const char* POST_PROCESS_VERTEX_SHADER_SOURCE = R"(
	#version 330

	// Input
	in vec3 positionIn;
	in vec2 texCoordIn;

	// Output
	out vec2 texCoord;

	void main()
	{
		gl_Position = vec4(positionIn, 1.0);
		texCoord = texCoordIn;
	}
)";

GLuint compilePostProcessShaderProgram(const char* vertexShaderSource) noexcept
{
	GLuint vertexShader = gl::compileVertexShader(POST_PROCESS_VERTEX_SHADER_SOURCE);

	GLuint fragmentShader = gl::compileFragmentShader(vertexShaderSource);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "positionIn");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

} // anoynous namespace

// Shader programs
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint compileShadowMapShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 positionIn;

		uniform mat4 uModelMatrix;
		uniform mat4 uViewMatrix;
		uniform mat4 uProjectionMatrix;

		void main()
		{
			mat4 modelViewProj = uProjectionMatrix * uViewMatrix * uModelMatrix;
			gl_Position = modelViewProj * vec4(positionIn, 1.0);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		out vec4 fragmentColor;

		void main()
		{
			fragmentColor = vec4(1.0);
		}
	)");

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "positionIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

GLuint compileGBufferGenShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		// Input
		in vec3 positionIn;
		in vec2 texCoordIn;
		in vec3 normalIn;

		// Output
		out vec2 texCoord;
		out vec3 vsPos;
		out vec3 vsNormal;

		// Uniforms
		uniform mat4 uModelMatrix;
		uniform mat4 uViewMatrix;
		uniform mat4 uProjectionMatrix;

		void main()
		{
			mat4 modelView = uViewMatrix * uModelMatrix;
			mat4 modelViewProj = uProjectionMatrix * modelView;
			mat4 normalMatrix = inverse(transpose(modelView)); // Needed for non-uniform scaling.

			// Output
			gl_Position = modelViewProj * vec4(positionIn, 1);
			texCoord = texCoordIn;
			vsPos = vec3(modelView * vec4(positionIn, 1));
			vsNormal = normalize((normalMatrix * vec4(normalIn, 0)).xyz);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // Required by GLSL spec Sect 4.5.3

		// Input
		in vec2 texCoord;
		in vec3 vsPos;
		in vec3 vsNormal;

		// Output
		layout(location = 0) out vec4 fragmentDiffuse;
		layout(location = 1) out vec4 fragmentPosition;
		layout(location = 2) out vec4 fragmentNormal;
		layout(location = 3) out vec4 fragmentEmissive;
		layout(location = 4) out vec4 fragmentMaterial;

		// Uniforms
		uniform sampler2D uDiffuseTexture;
		uniform sampler2D uEmissiveTexture;

		uniform int uHasEmissiveTexture = 0;
		uniform vec3 uEmissive = vec3(0.0, 0.0, 0.0);
		uniform vec3 uMaterial = vec3(1.0 /*ambient*/, 1.0 /*diffuse*/, 1.0 /*specular*/);

		void main()
		{
			fragmentDiffuse = vec4(texture(uDiffuseTexture, texCoord).rgb, 1.0);
			fragmentPosition = vec4(vsPos, 1.0);
			fragmentNormal = vec4(vsNormal, 1.0);
			if (uHasEmissiveTexture != 0) {
				vec4 texEmissive = texture(uEmissiveTexture, texCoord);
				fragmentEmissive = vec4(texEmissive.rgb * texEmissive.a, 1.0);
			} else {
				fragmentEmissive = vec4(uEmissive, 1.0);
			}
			fragmentMaterial = vec4(uMaterial, 1.0);
		}
	)");


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "positionIn");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindAttribLocation(shaderProgram, 2, "normalIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentDiffuse");
	glBindFragDataLocation(shaderProgram, 1, "fragmentPosition");
	glBindFragDataLocation(shaderProgram, 2, "fragmentNormal");
	glBindFragDataLocation(shaderProgram, 3, "fragmentEmissive");
	glBindFragDataLocation(shaderProgram, 4, "fragmentMaterial");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

GLuint compileLightingShaderProgram() noexcept
{
	return compilePostProcessShaderProgram(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3
		
		// Input
		in vec2 texCoord;

		// Output
		out vec4 fragmentColor;

		// Uniforms
		uniform sampler2D uDiffuseTexture;
		uniform sampler2D uPositionTexture;
		uniform sampler2D uNormalTexture;
		uniform sampler2D uEmissiveTexture;
		uniform sampler2D uMaterialTexture;
		uniform sampler2D uAOTexture;
		uniform sampler2DShadow uShadowMap;

		uniform mat4 uViewMatrix;

		uniform mat4 uLightMatrix;
		uniform vec3 uLightPos;
		uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);
		uniform float uLightRange;

		uniform float uLightShaftExposure = 0.4;

		float sampleShadowMap(vec3 vsSamplePos)
		{
			float shadow = 0.0;
			vec4 smCoord = uLightMatrix * vec4(vsSamplePos, 1.0);
			if (smCoord.z > 0.0) shadow = textureProj(uShadowMap, smCoord);
			return shadow;
		}

		float lightShaftFactor(vec3 vsPos, int numSamples, float maxSampleDist)
		{
			vec3 camDir = normalize(vsPos);
			float sampleLength = min(length(vsPos), maxSampleDist) / float(numSamples+1);
			vec3 toNextSamplePos = camDir * sampleLength;
	
			vec3 currentSamplePos = toNextSamplePos;
			float factor = 0.0;
			for (int i = 0; i < numSamples; i++) {
				factor += sampleShadowMap(currentSamplePos);
				currentSamplePos += toNextSamplePos;
			}
			factor /= float(numSamples);
			
			return factor;
		}

		void main()
		{
			// Constants (that should probably be externally defined)
			const float materialShininess = 6;
			const vec3 ambientLight = vec3(0.25);

			// Values from textures
			vec3 diffuseColor = texture(uDiffuseTexture, texCoord).rgb;
			vec3 vsPos = texture(uPositionTexture, texCoord).xyz;
			vec3 vsNormal = normalize(texture(uNormalTexture, texCoord).xyz);
			vec3 emissive = texture(uEmissiveTexture, texCoord).rgb;
			vec3 material = texture(uMaterialTexture, texCoord).xyz;
			float materialAmbient = material.x;
			float materialDiffuse = material.y;
			float materialSpecular = material.z;
			float ao = texture(uAOTexture, texCoord).r;
			float shadow = sampleShadowMap(vsPos);

			// Light calculation positions
			vec3 vsLightPos = (uViewMatrix * vec4(uLightPos, 1)).xyz;
			vec3 toLight = normalize(vsLightPos - vsPos);
			vec3 toCam = normalize(-vsPos);
			vec3 halfVec = normalize(toLight + toCam);

			// Light scaling
			float lightDist = length(vsLightPos - vsPos);
			//float lightScale = max((-1.0/uLightRange)*lightDist + 1.0, 0); // Linear
			float lightScale = max((-1.0/(uLightRange*uLightRange))
							 *(lightDist*lightDist - uLightRange*uLightRange), 0); // Quadratic

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

			vec3 shading = emissive
			             + materialAmbient * ambientLight * diffuseColor * ao
			             + materialDiffuse * diffuseLight * diffuseColor
						 + materialSpecular * specularLight
						 + uLightShaftExposure * lightShaftFactor(vsPos, 40, 25.0) * uLightColor * lightScale;

			fragmentColor = vec4(shading, 1.0);
		}
	)");
}

GLuint compileOutputSelectShaderProgram() noexcept
{
	return compilePostProcessShaderProgram(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		// Input
		in vec2 texCoord;

		// Output
		out vec4 fragmentColor;

		// Uniforms
		// Finished output
		uniform sampler2D uFinishedTexture;
		
		// GBuffer
		uniform sampler2D uDiffuseTexture;
		uniform sampler2D uPositionTexture;
		uniform sampler2D uNormalTexture;
		uniform sampler2D uEmissiveTexture;
		uniform sampler2D uMaterialTexture;
		
		// SSAO
		uniform sampler2D uAOTexture;

		uniform int uRenderMode;

		void main()
		{
			switch(uRenderMode) {
			case 1: fragmentColor = vec4(texture(uFinishedTexture, texCoord).rgb, 1.0); break;
			case 2: fragmentColor = vec4(texture(uDiffuseTexture, texCoord).rgb, 1.0); break;
			case 3: fragmentColor = vec4(texture(uPositionTexture, texCoord).xyz, 1.0); break;
			case 4: fragmentColor = vec4(texture(uNormalTexture, texCoord).xyz, 1.0); break;
			case 5: fragmentColor = vec4(texture(uEmissiveTexture, texCoord).rgb, 1.0); break;
			case 6: fragmentColor = vec4(texture(uMaterialTexture, texCoord).xyz, 1.0); break;
			case 7: fragmentColor = vec4(vec3(texture(uAOTexture, texCoord).r), 1.0); break;
			}
		}
	)");
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>