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

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;
		uniform vec3 msLightPos;

		void main()
		{
			mat4 modelView = viewMatrix * modelMatrix;
			mat4 modelViewProj = projectionMatrix * modelView;
			mat4 normalMatrix = inverse(transpose(modelView)); // Needed for non-uniform scaling.

			// Output
			gl_Position = modelViewProj * vec4(position, 1);
			texCoord = texCoordIn;
			vsNormal = normalize((normalMatrix * vec4(normalIn, 0)).xyz).xyz;
			vsLightPos = (viewMatrix * vec4(msLightPos, 1)).xyz;
			vsPos = vec3(modelView * vec4(position, 1));
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		in vec2 texCoord;
		in vec3 vsNormal;
		in vec3 vsLightPos;
		in vec3 vsPos;

		out vec4 fragmentColor;

		uniform sampler2D tex;
		uniform vec3 lightColor;

		// Constants
		vec3 ambientLight = vec3(0.05f, 0.05f, 0.05f);

		void main()
		{
			vec4 diffuseTexture = texture(tex, texCoord.xy);

			vec3 toLight = normalize(vsLightPos - vsPos);
			float cosTheta = clamp(dot(vsNormal, toLight), 0, 1);

			fragmentColor = vec4(ambientLight,1)*diffuseTexture + cosTheta*diffuseTexture;
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

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

} // namespace vox