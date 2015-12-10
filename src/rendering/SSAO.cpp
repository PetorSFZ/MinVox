#include "rendering/SSAO.hpp"

#include <sfz/gl/OpenGL.hpp>

namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

// SSAO shader
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* SSAO_FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 uvCoord;

	// Output
	out vec4 occlusionOut;

	// Constants
	const int MAX_KERNEL_SIZE = 256;

	// Uniforms
	uniform sampler2D uPositionTexture;
	uniform sampler2D uNormalTexture;
	uniform sampler2D uNoiseTexture;
		
	uniform int uKernelSize;
	uniform vec3 uKernel[MAX_KERNEL_SIZE];
	uniform mat4 uProjectionMatrix;

	uniform vec2 uNoiseTexCoordScale;
	uniform float uRadius;
	uniform float uOcclusionExp;

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
		// SSAO implementation using normal oriented hemisphere, inspired by this tutorial:
		// http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html

		vec3 vsPos = texture(uPositionTexture, uvCoord).xyz;
		vec3 normal = normalize(texture(uNormalTexture, uvCoord).xyz);
		float depth = vsPosToDepth(vsPos);
		vec3 noiseVec = texture(uNoiseTexture, uvCoord * uNoiseTexCoordScale).xyz;

		// Calculates matrix to rotate kernel into normal hemisphere using Gram Schmidt process
		vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
		vec3 bitangent = cross(tangent, normal);
		mat3 kernelRot = mat3(tangent, bitangent, normal);

		float occlusion = 0.0;
		for (int i = 0; i < uKernelSize; i++) {
			vec3 samplePos = vsPos + uRadius * (kernelRot * uKernel[i]);
			vec2 sampleTexCoord = texCoordFromVSPos(samplePos);
			float sampleDepth = vsPosToDepth(texture(uPositionTexture, sampleTexCoord).xyz);

			float rangeCheck = abs(depth - sampleDepth) < uRadius ? 1.0 : 0.0;
			occlusion += (sampleDepth <= depth ? 0.0 : 1.0) * rangeCheck;

			//float rangeCheck = abs(depth - sampleDepth) < uRadius ? 1.0 : 0.0;
			//occlusion += (sampleDepth <= samplePos.z ? 1.0 : 0.0) * rangeCheck;
			
			//float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(depth - sampleDepth));
			//occlusion += (step(sampleDepth, samplePos.z)); // * rangeCheck);
		}
		occlusion = 1.0 - (occlusion / uKernelSize);
		occlusion = pow(occlusion, uOcclusionExp);

		occlusionOut = vec4(occlusion, 0.0, 0.0, 1.0);
	}
)";

// SSAO blur shader
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* SSAO_BLUR_FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 uvCoord;
	
	// Output
	out vec4 fragmentColor;

	// Constants
	const int blurWidth = 4;
	const float blurWidthFloat = 4.0;

	// Uniforms
	uniform sampler2D uOcclusionTexture;

	void main()
	{
		// Blur shader from: http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
		// Used to blur away the noise applied during the first stage.

		vec2 texelSize = 1.0 / vec2(textureSize(uOcclusionTexture, 0));

		// Offset to the offset so we sample values around the current texcoord.
		vec2 offsetOffset = vec2(-blurWidthFloat * 0.5 + 0.5);

		float blur = 0.0;
		for (int x = 0; x < blurWidth; x++) {
			for (int y = 0; y < blurWidth; y++) {
				vec2 offset = vec2(float(x), float(y));
				offset += offsetOffset;
				offset *= texelSize;
				blur += texture(uOcclusionTexture, uvCoord + offset).r;
			}
		}
		blur /= (blurWidthFloat*blurWidthFloat);
		
		fragmentColor = vec4(vec3(blur), 1.0);
	}
)";

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

gl::Program compileSSAOShaderProgram() noexcept
{
	return gl::Program::postProcessFromSource(SSAO_FRAGMENT_SHADER);
}

gl::Program compileBlurShaderProgram() noexcept
{
	return gl::Program::postProcessFromSource(SSAO_BLUR_FRAGMENT_SHADER);
}

vector<vec3> generateKernel(size_t kernelSize) noexcept
{
	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr1{-1.0f, 1.0f};
	std::uniform_real_distribution<float> distr2{0.0f, 1.0f};

	vector<vec3> kernel{kernelSize};
	for (size_t i = 0; i < kernelSize; i++) {
		// Random vector in z+ hemisphere.
		kernel[i] = sfz::normalize(vec3{distr1(gen), distr1(gen), distr2(gen)});
		// Scale it so it has length between 0 and 1.
		//kernel[i] *= distr2(gen); // Naive solution
		// More points closer to base, see: http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
		float scale = (float)i / (float)kernelSize;
		scale = sfz::lerp(0.1f, 1.0f, scale*scale);
		kernel[i] *= scale;
	}

	/*std::cout << "Generated SSAO sample kernel (size = " << kernelSize << ") with values: \n";
	for (auto& val : kernel) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;*/

	return std::move(kernel);
}

GLuint generateNoiseTexture(size_t noiseTexWidth) noexcept
{
	static_assert(sizeof(vec3) == sizeof(float)*3, "vec3 is padded");

	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr{-1.0f, 1.0f};

	size_t numNoiseValues = noiseTexWidth*noiseTexWidth;
	vector<vec3> noise{numNoiseValues};
	for (size_t i = 0; i < numNoiseValues; i++) {
		noise[i] = vec3{distr(gen), distr(gen), 0.0f};
	}

	/*std::cout << "Generated SSAO noise texture (width = " << noiseTexWidth << ") with values: \n";
	for (auto& val : noise) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;*/

	GLuint noiseTex;
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, static_cast<GLsizei>(noiseTexWidth), noiseTex, 0,
	//             GL_RGB, GL_FLOAT, noise.data());
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, static_cast<GLsizei>(noiseTexWidth), static_cast<GLsizei>(noiseTexWidth), 0,
	             GL_RGB, GL_FLOAT, noise.data());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTex;
}

} // anonymous namespace

// Occlusion Framebuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

OcclusionFramebuffer::OcclusionFramebuffer(int width, int height) noexcept
:
	mWidth{width},
	mHeight{height}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	// Color texture
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // TODO: Dunno?
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OcclusionFramebuffer::OcclusionFramebuffer(OcclusionFramebuffer&& other) noexcept
{
	glGenFramebuffers(1, &mFBO);
	glGenTextures(1, &mTexture);

	std::swap(mFBO, other.mFBO);
	std::swap(mTexture, other.mTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
}

OcclusionFramebuffer& OcclusionFramebuffer::operator= (OcclusionFramebuffer&& other) noexcept
{
	std::swap(mFBO, other.mFBO);
	std::swap(mTexture, other.mTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
	return *this;
}

OcclusionFramebuffer::~OcclusionFramebuffer() noexcept
{
	glDeleteTextures(1, &mTexture);
	glDeleteFramebuffers(1, &mFBO);
}


// SSAO
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// SSAO: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SSAO::SSAO(int width, int height, size_t numSamples, float radius, float occlusionExp) noexcept
:
	mWidth{width},
	mHeight{height},
	mSSAOProgram{compileSSAOShaderProgram()},
	mBlurProgram{compileBlurShaderProgram()},
	mOcclusionFBO{mWidth, mHeight},
	mBlurredFBO{mWidth, mHeight},
	mKernelSize{numSamples > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : numSamples},
	mKernel(std::move(generateKernel(MAX_KERNEL_SIZE))),
	mNoiseTexWidth{4},
	mNoiseTexture{generateNoiseTexture(mNoiseTexWidth)},
	mRadius{radius},
	mOcclusionExp{occlusionExp}
{ }

SSAO::~SSAO() noexcept
{
	glDeleteTextures(1, &mNoiseTexture);
}

// SSAO: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint SSAO::calculate(GLuint posTex, GLuint normalTex, const mat4& projMatrix, bool clean) noexcept
{
	// Render occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mSSAOProgram.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mOcclusionFBO.mFBO);
	glViewport(0, 0, mOcclusionFBO.mWidth, mOcclusionFBO.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture buffer uniforms

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posTex);
	gl::setUniform(mSSAOProgram, "uPositionTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTex);
	gl::setUniform(mSSAOProgram, "uNormalTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);
	gl::setUniform(mSSAOProgram, "uNoiseTexture", 2);

	// Other uniforms

	gl::setUniform(mSSAOProgram, "uProjectionMatrix", projMatrix);

	gl::setUniform(mSSAOProgram, "uKernelSize", static_cast<int>(mKernelSize));
	gl::setUniform(mSSAOProgram, "uKernel", static_cast<vec3*>(mKernel.data()), mKernelSize);

	gl::setUniform(mSSAOProgram, "uNoiseTexCoordScale",
	     vec2{(float)mWidth, (float)mHeight} / (float)mNoiseTexWidth);
	gl::setUniform(mSSAOProgram, "uRadius", mRadius);
	gl::setUniform(mSSAOProgram, "uOcclusionExp", mOcclusionExp);

	mPostProcessQuad.render();

	// Blur occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	if (clean) {
		glUseProgram(mBlurProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mBlurredFBO.mFBO);
		glViewport(0, 0, mBlurredFBO.mWidth, mBlurredFBO.mHeight);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mOcclusionFBO.mTexture);
		gl::setUniform(mBlurProgram, "uOcclusionTexture", 0);

		mPostProcessQuad.render();

		return mBlurredFBO.mTexture;
	} else {
		return mOcclusionFBO.mTexture;
	}
}

// SSAO: Getters / setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::textureSize(int width, int height) noexcept
{
	mWidth = width;
	mHeight = height;
	mOcclusionFBO = OcclusionFramebuffer{mWidth, mHeight};
	mBlurredFBO = OcclusionFramebuffer{mWidth, mHeight};
}

void SSAO::numSamples(size_t numSamples) noexcept
{
	if (numSamples > MAX_KERNEL_SIZE) return;
	mKernelSize = numSamples;
}

void SSAO::radius(float radius) noexcept
{
	if (radius <= 0.0f) return;
	mRadius = radius;
}

void SSAO::occlusionExp(float occlusionExp) noexcept
{
	if (occlusionExp <= 0.0f) return;
	mOcclusionExp = occlusionExp;
}

} // namespace vox