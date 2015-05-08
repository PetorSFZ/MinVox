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

		// Uniforms
		uniform sampler2D uDiffuseTexture;

		void main()
		{
			fragmentDiffuse = vec4(texture(uDiffuseTexture, texCoord).rgb, 1.0);
			fragmentPosition = vec4(vsPos, 1.0);
			fragmentNormal = vec4(vsNormal, 1.0);
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
		uniform sampler2D uOcclusionTexture;
		uniform sampler2DShadow uShadowMap;

		uniform mat4 uViewMatrix;

		uniform mat4 uLightMatrix;
		uniform vec3 uLightPos;
		uniform vec3 uLightColor;

		uniform float uLightShaftExposure = 0.4;

		float lightShaftFactor(vec3 vsPos, int numSamples)
		{
			vec3 camDir = normalize(vsPos);
			float sampleLength = length(vsPos) / float(numSamples+1);
			vec3 toNextSamplePos = camDir * sampleLength;
	
			vec3 currentSamplePos = toNextSamplePos;
			float factor = 0.0;
			for (int i = 0; i < numSamples; i++) {
				vec4 smCoord = uLightMatrix * vec4(currentSamplePos, 1.0);
				float temp;
				if (smCoord.z <= 0) temp = 0.0;
				else temp = textureProj(uShadowMap, smCoord);
				factor += temp;
				currentSamplePos += toNextSamplePos;
			}
			factor /= float(numSamples);
			
			return factor;
		}

		void main()
		{
			// Values from textures
			vec3 diffuseColor = texture(uDiffuseTexture, texCoord).rgb;
			vec3 vsPos = texture(uPositionTexture, texCoord).xyz;
			vec3 vsNormal = normalize(texture(uNormalTexture, texCoord).xyz);
			float occlusion = texture(uOcclusionTexture, texCoord).r;

			vec3 vsLightPos = (uViewMatrix * vec4(uLightPos, 1)).xyz;
			vec4 shadowMapCoord = uLightMatrix * vec4(vsPos, 1.0);

			// Texture and materials
			vec3 ambientLight = vec3(0.35, 0.35, 0.35);
			vec3 materialAmbient = vec3(1.0, 1.0, 1.0) * diffuseColor;
			vec3 materialDiffuse = vec3(0.3, 0.3, 0.3) * diffuseColor;
			vec3 materialSpecular = vec3(0.35, 0.35, 0.35);
			vec3 materialEmissive = vec3(0, 0, 0);
			float materialShininess = 8;

			// Variables used to calculate scaling factors for different components
			vec3 toLight = normalize(vsLightPos - vsPos);
			float lightDistance = length(vsLightPos - vsPos);
			vec3 toCam = normalize(-vsPos);
			vec3 halfVec = normalize(toLight + toCam);
			float specularNormalization = ((materialShininess + 2.0) / 8.0);
			
			float temp;
			if (shadowMapCoord.z <= 0) temp = 0.0;
			else temp = textureProj(uShadowMap, shadowMapCoord);
			float lightVisibility = temp;
			float lightShafts = lightShaftFactor(vsPos, 42);

			// Scaling factors for different components
			float diffuseFactor = clamp(dot(vsNormal, toLight), 0, 1);
			float specularFactor = specularNormalization * pow(clamp(dot(vsNormal, halfVec), 0, 1), materialShininess);

			// Calculate shading
			vec3 shading = ambientLight * materialAmbient * occlusion
			             + diffuseFactor * materialDiffuse * uLightColor * lightVisibility
			             + specularFactor * materialSpecular * uLightColor * lightVisibility
			             + materialEmissive
			             + uLightShaftExposure * lightShafts * uLightColor;

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
		
		// SSAO
		uniform sampler2D uOcclusionTexture;

		uniform int uRenderMode;

		void main()
		{
			switch(uRenderMode) {
			case 1: fragmentColor = vec4(texture(uFinishedTexture, texCoord).rgb, 1.0); break;
			case 2: fragmentColor = vec4(texture(uDiffuseTexture, texCoord).rgb, 1.0); break;
			case 3: fragmentColor = vec4(texture(uPositionTexture, texCoord).xyz, 1.0); break;
			case 4: fragmentColor = vec4(texture(uNormalTexture, texCoord).xyz, 1.0); break;
			case 5: fragmentColor = vec4(vec3(texture(uOcclusionTexture, texCoord).r), 1.0); break;
			}
		}
	)");
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>