#version 330

in vec3 inPosition;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	mat4 modelViewProj = uProjectionMatrix * uViewMatrix * uModelMatrix;
	gl_Position = modelViewProj * vec4(inPosition, 1.0);
}