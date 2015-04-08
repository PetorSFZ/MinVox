#include "rendering/Shaders.hpp"

namespace vox {

GLuint compileStandardShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 position;
		in vec2 texCoordIn;
		in vec3 normalIn;

		out vec2 texCoord;
		out vec3 vsNormal;
		out vec3 vsLightPos;
		out vec3 vsPos;
		out vec4 shadowMapCoord;

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;
		uniform mat4 lightMatrix;
		uniform vec3 msLightPos;
		uniform vec3 lightColor;

		void main()
		{
			mat4 modelView = viewMatrix * modelMatrix;
			mat4 modelViewProj = projectionMatrix * modelView;
			mat4 normalMatrix = inverse(transpose(modelView)); // Needed for non-uniform scaling.

			// This step needs to be done in shader since I don't have function to inverse 4x4 matrix in SFZ Common yet.
			mat4 lightMatrixComplete = lightMatrix * inverse(viewMatrix);

			// Output
			gl_Position = modelViewProj * vec4(position, 1);
			texCoord = texCoordIn;
			vsNormal = normalize((normalMatrix * vec4(normalIn, 0)).xyz);
			vsLightPos = (viewMatrix * vec4(msLightPos, 1)).xyz;
			vsPos = vec3(modelView * vec4(position, 1));
			shadowMapCoord = lightMatrixComplete * vec4(vsPos, 1.0);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		in vec2 texCoord;
		in vec3 vsNormal;
		in vec3 vsLightPos;
		in vec3 vsPos;
		in vec4 shadowMapCoord;

		layout(location = 0) out vec4 fragmentColor;
		layout(location = 1) out vec4 fragmentNormal;
		layout(location = 2) out vec4 fragmentPosition;

		uniform sampler2D tex;
		uniform sampler2DShadow shadowMap;
		uniform vec3 lightColor;

		void main()
		{
			// Texture and materials
			vec3 ambientLight = vec3(0.15, 0.15, 0.15);
			vec3 diffuseTexture = texture(tex, texCoord.xy).xyz;
			vec3 materialAmbient = vec3(1.0, 1.0, 1.0) * diffuseTexture;
			vec3 materialDiffuse = vec3(0.5, 0.5, 0.5) * diffuseTexture;
			vec3 materialSpecular = vec3(0.35, 0.35, 0.35);
			vec3 materialEmissive = vec3(0, 0, 0);
			float materialShininess = 8;

			// Variables used to calculate scaling factors for different components
			vec3 toLight = normalize(vsLightPos - vsPos);
			float lightDistance = length(vsLightPos - vsPos);
			vec3 toCam = normalize(-vsPos);
			vec3 halfVec = normalize(toLight + toCam);
			float specularNormalization = ((materialShininess + 2.0) / 8.0);
			float lightVisibility = textureProj(shadowMap, shadowMapCoord);

			// Scaling factors for different components
			float diffuseFactor = clamp(dot(vsNormal, toLight), 0, 1);
			float specularFactor = specularNormalization * pow(clamp(dot(vsNormal, halfVec), 0, 1), materialShininess);

			// Calculate shading
			vec3 shading = ambientLight * materialAmbient
			             + diffuseFactor * materialDiffuse * lightColor * lightVisibility
			             + specularFactor * materialSpecular * lightColor * lightVisibility
			             + materialEmissive;

			fragmentColor = vec4(shading, 1.0);
			fragmentNormal = vec4(vsNormal, 1.0);
			fragmentPosition = vec4(vsPos, 1.0);
		}
	)");


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindAttribLocation(shaderProgram, 2, "normalIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
	glBindFragDataLocation(shaderProgram, 1, "fragmentNormal");
	glBindFragDataLocation(shaderProgram, 2, "fragmentPosition");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

GLuint compileShadowMapShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec3 position;

		uniform mat4 modelMatrix;
		uniform mat4 viewMatrix;
		uniform mat4 projectionMatrix;

		void main()
		{
			mat4 modelViewProj = projectionMatrix * viewMatrix * modelMatrix;
			gl_Position = modelViewProj * vec4(position, 1.0);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		out vec4 fragmentColor;

		void main()
		{
			fragmentColor = vec4(1.0);
		}
	)");

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

GLuint compilePostProcessShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec2 position;

		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
		}
	)");


	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		out vec4 fragmentColor;

		uniform sampler2DRect colorTexture;
		uniform sampler2DRect normalTexture;
		uniform sampler2DRect depthTexture;
		uniform sampler2DRect positionTexture;
		
		uniform mat4 projectionMatrix;

		float linearizeDepth(float depth)
		{
			float near = 0.25; // camera z-near
			float far = 1000.0; // camera z-far
			return (2.0 * near) / (far + near - (depth*(far-near)));
		}

		// http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
		float rand(vec2 co){
			return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
		}

		void main()
		{
			vec2 textureCoord = gl_FragCoord.xy;
			vec4 color = texture(colorTexture, textureCoord);
			vec3 normal = normalize((texture(normalTexture, textureCoord).xyz * 2.0) - 1.0);
			float depth = texture(depthTexture, textureCoord).r;
			float linearDepth = linearizeDepth(depth);
			vec3 vsPos = texture(positionTexture, textureCoord).rgb;

			int kernelSize = 16;
			vec3 kernel[16] = vec3[]( vec3(0.53812504, 0.18565957, -0.43192),
			                          vec3(0.13790712, 0.24864247, 0.44301823),
			                          vec3(0.33715037, 0.56794053, -0.005789503),
			                          vec3(-0.6999805, -0.04511441, -0.0019965635),
			                          vec3(0.06896307, -0.15983082, -0.85477847),
			                          vec3(0.056099437, 0.006954967, -0.1843352),
			                          vec3(-0.014653638, 0.14027752, 0.0762037),
			                          vec3(0.010019933, -0.1924225, -0.034443386),
			                          vec3(-0.35775623, -0.5301969, -0.43581226),
			                          vec3(-0.3169221, 0.106360726, 0.015860917),
			                          vec3(0.010350345, -0.58698344, 0.0046293875),
			                          vec3(-0.08972908, -0.49408212, 0.3287904),
			                          vec3(0.7119986, -0.0154690035, -0.09183723),
			                          vec3(-0.053382345, 0.059675813, -0.5411899),
			                          vec3(0.035267662, -0.063188605, 0.54602677),
			                          vec3(-0.47761092, 0.2847911, -0.0271716) );

			float occlusion = 0.0f;

			for (int i = 0; i < kernelSize; i++) {
				vec3 samplePos = vsPos + kernel[i] * 10.0;
				
				vec4 sampleOffset = vec4(samplePos, 1.0);
				sampleOffset = projectionMatrix * sampleOffset;
				sampleOffset.xy /= sampleOffset.w;
				sampleOffset.xy = sampleOffset.xy * 0.5 + 0.5;

				float sampleDepth = texture(depthTexture, sampleOffset.xy).r;
				float sampleLinearDepth = linearizeDepth(sampleDepth);

				occlusion += (sampleLinearDepth <= samplePos.z ? 1.0 : 0.0) * (1.0/16.0) * 2.0;
			}

			if (textureCoord.x > 600) fragmentColor = vec4(vec3(occlusion), 1.0);
			else fragmentColor = occlusion * color;

			/*if (textureCoord.x > 600 && textureCoord.y > 600) {
				fragmentColor = vec4(vec3(linearDepth), 1.0);
			} else if (textureCoord.x > 600 && textureCoord.y < 600) {
				fragmentColor = vec4(normal, 1.0);
			} else {
				fragmentColor = color;
			}*/
		}
	)");

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}
	return shaderProgram;
}

} // namespace vox