#version 330

in vec3 inPosition;

uniform mat4 uModelMatrix;
uniform mat4 uViewProjMatrix;

void main()
{
	mat4 modelViewProj = uViewProjMatrix * uModelMatrix;
	gl_Position = modelViewProj * vec4(inPosition, 1.0);
}