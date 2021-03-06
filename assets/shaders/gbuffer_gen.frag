#version 330

in vec3 vsPos;
in vec3 vsNormal;
in vec2 uvCoord;

uniform sampler2D uDiffuseTexture;
uniform vec3 uMaterial = vec3(1.0 /*ambient*/, 1.0 /*diffuse*/, 1.0 /*specular*/);
uniform float uFarPlaneDist;

layout(location = 0) out vec4 outFragLinearDepth;
layout(location = 1) out vec4 outFragNormal;
layout(location = 2) out vec4 outFragDiffuse;
layout(location = 3) out vec4 outFragMaterial;

void main()
{
	outFragLinearDepth = vec4(-vsPos.z / uFarPlaneDist, 0.0, 0.0, 1.0);
	outFragNormal = vec4(normalize(vsNormal), 0.0);
	outFragDiffuse = texture(uDiffuseTexture, uvCoord);
	outFragMaterial = vec4(uMaterial, 1.0);
}