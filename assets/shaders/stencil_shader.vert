#version 330

// Input
in vec3 inPosition;

// Uniforms
uniform mat4 uModelMatrix;
uniform mat4 uViewProjMatrix;

void main()
{
	mat4 modelViewProj = uViewProjMatrix * uModelMatrix;
	gl_Position = modelViewProj * vec4(inPosition, 1.0);
}