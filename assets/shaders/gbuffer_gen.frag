#version 330

precision highp float; // Required by GLSL spec Sect 4.5.3

// Input
in vec2 texCoord;
in vec3 vsPos;
in vec3 vsNormal;

// Output
layout(location = 0) out vec4 fragmentDiffuse;
layout(location = 1) out vec4 fragmentPosition;
layout(location = 2) out vec4 fragmentNormal;
layout(location = 3) out vec4 fragmentEmissive;
layout(location = 4) out vec4 fragmentMaterial;

// Uniforms
uniform sampler2D uDiffuseTexture;
uniform sampler2D uEmissiveTexture;

uniform int uHasEmissiveTexture = 0;
uniform vec3 uEmissive = vec3(0.0, 0.0, 0.0);
uniform vec3 uMaterial = vec3(1.0 /*ambient*/, 1.0 /*diffuse*/, 1.0 /*specular*/);

void main()
{
	fragmentDiffuse = vec4(texture(uDiffuseTexture, texCoord).rgb, 1.0);
	fragmentPosition = vec4(vsPos, 1.0);
	fragmentNormal = vec4(vsNormal, 1.0);
	if (uHasEmissiveTexture != 0) {
		vec4 texEmissive = texture(uEmissiveTexture, texCoord);
		fragmentEmissive = vec4(texEmissive.rgb * texEmissive.a, 1.0);
	} else {
		fragmentEmissive = vec4(uEmissive, 1.0);
	}
	fragmentMaterial = vec4(uMaterial, 1.0);
}