#include "rendering/Shaders.hpp"

namespace vox {

GLuint compileStandardShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 position;
		in vec2 texCoordIn;
		in vec3 normalIn;

		out vec2 texCoord;
		out vec3 vsNormal;
		out vec3 vsLightPos;
		out vec3 vsPos;
		out vec4 shadowMapCoord;

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;
		uniform mat4 lightMatrix;
		uniform vec3 msLightPos;
		uniform vec3 lightColor;

		void main()
		{
			mat4 modelView = viewMatrix * modelMatrix;
			mat4 modelViewProj = projectionMatrix * modelView;
			mat4 normalMatrix = inverse(transpose(modelView)); // Needed for non-uniform scaling.

			// This step needs to be done in shader since I don't have function to inverse 4x4 matrix in SFZ Common yet.
			mat4 lightMatrixComplete = lightMatrix * inverse(viewMatrix);

			// Output
			gl_Position = modelViewProj * vec4(position, 1);
			texCoord = texCoordIn;
			vsNormal = normalize((normalMatrix * vec4(normalIn, 0)).xyz);
			vsLightPos = (viewMatrix * vec4(msLightPos, 1)).xyz;
			vsPos = vec3(modelView * vec4(position, 1));
			shadowMapCoord = lightMatrixComplete * vec4(vsPos, 1.0);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		in vec2 texCoord;
		in vec3 vsNormal;
		in vec3 vsLightPos;
		in vec3 vsPos;
		in vec4 shadowMapCoord;

		layout(location = 0) out vec4 fragmentColor;
		layout(location = 1) out vec4 fragmentNormal;
		layout(location = 2) out vec4 fragmentPosition;

		uniform sampler2D tex;
		uniform sampler2DShadow shadowMap;
		uniform vec3 lightColor;

		void main()
		{
			// Texture and materials
			vec3 ambientLight = vec3(0.15, 0.15, 0.15);
			vec3 diffuseTexture = texture(tex, texCoord.xy).xyz;
			vec3 materialAmbient = vec3(1.0, 1.0, 1.0) * diffuseTexture;
			vec3 materialDiffuse = vec3(0.5, 0.5, 0.5) * diffuseTexture;
			vec3 materialSpecular = vec3(0.35, 0.35, 0.35);
			vec3 materialEmissive = vec3(0, 0, 0);
			float materialShininess = 8;

			// Variables used to calculate scaling factors for different components
			vec3 toLight = normalize(vsLightPos - vsPos);
			float lightDistance = length(vsLightPos - vsPos);
			vec3 toCam = normalize(-vsPos);
			vec3 halfVec = normalize(toLight + toCam);
			float specularNormalization = ((materialShininess + 2.0) / 8.0);
			float lightVisibility = textureProj(shadowMap, shadowMapCoord);

			// Scaling factors for different components
			float diffuseFactor = clamp(dot(vsNormal, toLight), 0, 1);
			float specularFactor = specularNormalization * pow(clamp(dot(vsNormal, halfVec), 0, 1), materialShininess);

			// Calculate shading
			vec3 shading = ambientLight * materialAmbient
			             + diffuseFactor * materialDiffuse * lightColor * lightVisibility
			             + specularFactor * materialSpecular * lightColor * lightVisibility
			             + materialEmissive;

			fragmentColor = vec4(shading, 1.0);
			fragmentNormal = vec4(vsNormal, 0.0);
			fragmentPosition = vec4(vsPos, 1.0);
		}
	)");


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindAttribLocation(shaderProgram, 2, "normalIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
	glBindFragDataLocation(shaderProgram, 1, "fragmentNormal");
	glBindFragDataLocation(shaderProgram, 2, "fragmentPosition");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

GLuint compileShadowMapShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 position;

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;

		void main()
		{
			mat4 modelViewProj = projectionMatrix * viewMatrix * modelMatrix;
			gl_Position = modelViewProj * vec4(position, 1.0);
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

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

} // namespace vox