#version 330

// Input
in vec3 positionIn;

// Uniforms
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	mat4 modelViewProj = uProjectionMatrix * uViewMatrix * uModelMatrix;
	gl_Position = modelViewProj * vec4(positionIn, 1.0);
}