#version 330

in vec3 inPosition;
in vec3 inNormal;
in vec2 inUVCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;

out vec3 vsPos;
out vec3 vsNormal;
out vec2 uvCoord;

void main()
{
	mat4 modelView = uViewMatrix * uModelMatrix;
	mat4 modelViewProj = uProjMatrix * modelView;
	mat4 normalMatrix = inverse(transpose(modelView)); // Needed for non-uniform scaling.

	gl_Position = modelViewProj * vec4(inPosition, 1);	
	vsPos = vec3(modelView * vec4(inPosition, 1));
	vsNormal = normalize((normalMatrix * vec4(inNormal, 0)).xyz);
	uvCoord = inUVCoord;
}