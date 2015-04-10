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

		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
		}
	)");

	GLuint fragmentShader = gl::compileFragmentShader(R"(
		#version 330

		precision highp float; // required by GLSL spec Sect 4.5.3

		// Output
		out vec4 fragmentColor;

		// Constants
		const int MAX_KERNEL_SIZE = 128;

		// Uniforms
		uniform sampler2DRect colorTexture;
		uniform sampler2DRect depthTexture;		
		uniform sampler2DRect normalTexture;
		uniform sampler2DRect positionTexture;
		
		uniform int kernelSize;
		uniform vec3 kernel[MAX_KERNEL_SIZE];
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
			float depth = texture(depthTexture, textureCoord).r;
			float linearDepth = linearizeDepth(depth);
			
			// TODO: Very unsure how to read vectors from textures correctly.
			vec3 normal = normalize((texture(normalTexture, textureCoord).xyz * 2.0) - 1.0);
			//vec3 vsPos = (texture(positionTexture, textureCoord).rgb * 2.0) - 1.0;
			vec3 vsPos = texture(positionTexture, textureCoord).rgb;

			
			vec3 noiseVec = vec3(0.5,0.5,0); // Note, should really come from tiled noise texture.

			// http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
			vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
			vec3 bitangent = cross(normal, tangent);
			mat3 tbn = mat3(tangent, bitangent, normal);

			float radius = 1;
			float occlusion = 0.0;
			for (int i = 0; i < kernelSize; i++) {
				vec3 samplePos = vsPos + radius * (tbn * kernel[i]);
				
				vec4 offset = projectionMatrix * vec4(samplePos, 1.0);
				offset.xy /= offset.w;
				offset.xy = offset.xy * 0.5 + 0.5;

				float sampleLinDepth = linearizeDepth(texture(depthTexture, offset.xy).r);
				
				float rangeCheck = abs(vsPos.z - sampleLinDepth) < radius ? 1.0 : 0.0;
				occlusion += (sampleLinDepth <= samplePos.z ? 1.0 : 0.0) * rangeCheck;
			}
			
			occlusion /= kernelSize;


			/*float occlusion = 0.0;
			
			for (int i = 0; i < kernelSize; i++) {
				vec3 samplePos = vsPos + kernel[i] * 0.1;
				
				vec4 sampleOffset = vec4(samplePos, 1.0);
				sampleOffset = projectionMatrix * sampleOffset;
				sampleOffset.xy /= sampleOffset.w;
				sampleOffset.xy = sampleOffset.xy * 0.5 + 0.5;

				float sampleDepth = texture(depthTexture, sampleOffset.xy).r;
				float sampleLinearDepth = linearizeDepth(sampleDepth);

				occlusion += (sampleLinearDepth <= samplePos.z ? 1.0 : 0.0) * (1.0/16.0);
			}*/

			if (textureCoord.x < 225) fragmentColor = vec4(vec3(occlusion), 1.0);
			else if (textureCoord.x < 450) fragmentColor = vec4(vsPos, 1.0);
			else if (textureCoord.x < 675) fragmentColor = vec4(normal, 1.0);
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

SSAO::SSAO(size_t kernelSize) noexcept
:
	mSSAOProgram{compileSSAOShaderProgram()},
	mKernelSize{kernelSize > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : kernelSize},
	mKernel{std::move(generateKernel(mKernelSize))}
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
	glBindTexture(GL_TEXTURE_RECTANGLE, colorTex);
	gl::setUniform(mSSAOProgram, "colorTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, depthTex);
	gl::setUniform(mSSAOProgram, "depthTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, normalTex);
	gl::setUniform(mSSAOProgram, "normalTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, posTex);
	gl::setUniform(mSSAOProgram, "positionTexture", 3);

	// Other uniforms

	gl::setUniform(mSSAOProgram, "projectionMatrix", projectionMatrix);

	gl::setUniform(mSSAOProgram, "kernelSize", static_cast<int>(mKernelSize));
	gl::setUniform(mSSAOProgram, "kernel", static_cast<vec3f*>(mKernel.data()), mKernelSize);

	mFullscreenQuad.render();
}

} // namespace vox

#include "sfz/MSVC12HackOFF.hpp"