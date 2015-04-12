#include "rendering/SSAO.hpp"

#include "sfz/MSVC12HackON.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

GLuint compileSSAOShaderProgram()
{
	GLuint vertexShader = gl::compileVertexShader(R"(
		#version 330

		in vec2 position;
		in vec2 texCoordIn;

		out vec2 texCoord;

		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
			texCoord = texCoordIn;
		}
	)");

	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		// Input
		in vec2 texCoord;

		// Output
		out vec4 fragmentColor;

		// Constants
		const int MAX_KERNEL_SIZE = 128;

		// Uniforms
		uniform sampler2D uColorTexture;
		uniform sampler2D uDepthTexture;
		uniform sampler2D uNormalTexture;
		uniform sampler2D uPositionTexture;
		
		uniform int uKernelSize;
		uniform vec3 uKernel[MAX_KERNEL_SIZE];
		uniform mat4 uProjectionMatrix;

		uniform float uRadius;

		float linearizeDepth(float depth)
		{
			/*float near = 0.5; // camera z-near
			float far = 1000.0; // camera z-far
			return (2.0 * near) / (far + near - (depth*(far-near)));*/
			
			// http://stackoverflow.com/questions/7777913/how-to-render-depth-linearly-in-modern-opengl-with-gl-fragcoord-z-in-fragment-sh/16597492#16597492
			vec4 unprojected = vec4(0, 0, depth * 2.0 - 1.0, 1.0);
			vec4 projected = inverse(uProjectionMatrix) * unprojected;
			projected /= projected.w;
			return projected.z;
		}

		float vsPosToDepth(vec3 vsPos)
		{
			float depth = vsPos.z;
			//if (depth >= 0) depth = -1000000.0;
			return depth;
		}

		vec2 texCoordFromVSPos(vec3 vsPos)
		{
			vec4 offset = uProjectionMatrix * vec4(vsPos, 1.0);
			offset.xy /= offset.w;
			offset.xy = offset.xy * 0.5 + 0.5;
			return offset.xy;
		}

		void main()
		{
			vec2 textureCoord = texCoord;
			vec4 color = texture(uColorTexture, textureCoord);
			vec3 normal = normalize(texture(uNormalTexture, textureCoord).rgb);
			vec3 vsPos = texture(uPositionTexture, textureCoord).rgb;
			float depth = vsPosToDepth(vsPos);
			//float depth = linearizeDepth(texture(depthTexture, textureCoord).r);

			vec3 noiseVec = vec3(0.0, 1.0, 0); // Note, should really come from tiled noise texture.

			// http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
			vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
			vec3 bitangent = cross(tangent, normal);
			mat3 kernelRot = mat3(tangent, bitangent, normal);

			float occlusion = 0.0;
			for (int i = 0; i < uKernelSize; i++) {
				vec3 samplePos = vsPos + uRadius * (kernelRot * uKernel[i]);
				vec2 sampleTexCoord = texCoordFromVSPos(samplePos);
				float sampleDepth = vsPosToDepth(texture(uPositionTexture, sampleTexCoord).xyz);

				float rangeCheck = abs(vsPos.z - sampleDepth) < uRadius ? 1.0 : 0.0;
				occlusion += (sampleDepth <= samplePos.z ? 1.0 : 0.0) * rangeCheck;
			}
			occlusion /= uKernelSize;
			
			if (textureCoord.x < 0.3) fragmentColor = color;
			else if (textureCoord.x < 0.6) fragmentColor = vec4(vec3(occlusion), 1.0);
			else fragmentColor = occlusion * color;
		}
	)");

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}

	return shaderProgram;
}

// Lerp function from wikipedia: http://en.wikipedia.org/wiki/Lerp_%28computing%29
// Precise method which guarantees v = v1 when t = 1.
float lerp(float v0, float v1, float t)
{
	return (1-t)*v0 + t*v1;
}

vector<vec3f> generateKernel(size_t kernelSize)
{
	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr1{-1.0f, 1.0f};
	std::uniform_real_distribution<float> distr2{0.0f, 1.0f};

	vector<vec3f> kernel{kernelSize};
	for (size_t i = 0; i < kernelSize; i++) {
		// Random vector in z+ hemisphere.
		kernel[i] = vec3f{distr1(gen), distr1(gen), distr2(gen)}.normalize();
		// Scale it so it has length between 0 and 1.
		//kernel[i] *= distr2(gen); // Naive solution
		// More points closer to base, see: http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
		float scale = (float)i / (float)kernelSize;
		scale = lerp(0.1f, 1.0f, scale*scale);
		kernel[i] *= scale;
	}

	return std::move(kernel);
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SSAO::SSAO(size_t numSamples, float radius) noexcept
:
	mSSAOProgram{compileSSAOShaderProgram()},
	mKernelSize{numSamples > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : numSamples},
	mKernel{std::move(generateKernel(mKernelSize))},
	mRadius{radius}
{ }

SSAO::~SSAO() noexcept
{
	glDeleteProgram(mSSAOProgram);
}

// Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::apply(GLuint targetFramebuffer, int framebufferWidth, int framebufferHeight,
                 GLuint colorTex, GLuint depthTex, GLuint normalTex, GLuint posTex,
                 const mat4f& projectionMatrix) noexcept
{
	glUseProgram(mSSAOProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	glViewport(0, 0, framebufferWidth, framebufferHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture buffer uniforms

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	gl::setUniform(mSSAOProgram, "uColorTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	gl::setUniform(mSSAOProgram, "uDepthTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTex);
	gl::setUniform(mSSAOProgram, "uNormalTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, posTex);
	gl::setUniform(mSSAOProgram, "uPositionTexture", 3);

	// Other uniforms

	gl::setUniform(mSSAOProgram, "uProjectionMatrix", projectionMatrix);

	gl::setUniform(mSSAOProgram, "uKernelSize", static_cast<int>(mKernelSize));
	gl::setUniform(mSSAOProgram, "uKernel", static_cast<vec3f*>(mKernel.data()), mKernelSize);

	gl::setUniform(mSSAOProgram, "uRadius", mRadius);

	mFullscreenQuad.render();
}

} // namespace vox

#include "sfz/MSVC12HackOFF.hpp"