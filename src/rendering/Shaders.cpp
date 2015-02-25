#include "rendering/Shaders.hpp"

namespace vox {

GLuint compileStandardShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 position;
		in vec2 texCoordIn;

		out vec2 texCoord;
		out vec3 vsLightPos;

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;
		uniform vec3 msLightPos;

		void main()
		{
			mat4 modelViewProj = projectionMatrix * viewMatrix * modelMatrix;

			gl_Position = modelViewProj * vec4(position, 1);
			texCoord = texCoordIn;
			vsLightPos = (viewMatrix * vec4(msLightPos, 1)).xyz;
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		in vec2 texCoord;
		in vec3 vsLightPos;

		out vec4 fragmentColor;

		uniform sampler2D tex;
		uniform vec3 lightColor;

		void main()
		{
			fragmentColor = texture(tex, texCoord.xy);
		}
	)");


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

} // namespace vox