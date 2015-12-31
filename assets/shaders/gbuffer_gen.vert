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