#version 330

// Input
in vec2 uvCoord;

// Output
out vec4 fragmentColor;

// Uniforms
// Finished output
uniform sampler2D uFinishedTexture;

// GBuffer
uniform sampler2D uDiffuseTexture;
uniform sampler2D uPositionTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uMaterialTexture;
uniform sampler2D uDirectionalLightsTexture;

// SSAO
uniform sampler2D uAOTexture;

uniform int uRenderMode;

void main()
{
	switch(uRenderMode) {
	case 1: fragmentColor = vec4(texture(uFinishedTexture, uvCoord).rgb, 1.0); break;
	case 2: fragmentColor = vec4(texture(uDiffuseTexture, uvCoord).rgb, 1.0); break;
	case 3: fragmentColor = vec4(texture(uPositionTexture, uvCoord).xyz, 1.0); break;
	case 4: fragmentColor = vec4(texture(uNormalTexture, uvCoord).xyz, 1.0); break;
	case 5: fragmentColor = vec4(texture(uEmissiveTexture, uvCoord).rgb, 1.0); break;
	case 6: fragmentColor = vec4(texture(uMaterialTexture, uvCoord).xyz, 1.0); break;
	case 7: fragmentColor = vec4(vec3(texture(uAOTexture, uvCoord).r), 1.0); break;
	case 8: fragmentColor = vec4(texture(uDirectionalLightsTexture, uvCoord).rgb, 1.0); break;
	}
}