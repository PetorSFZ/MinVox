#include "rendering/SSAO.hpp"

#include <random>

#include <sfz/Assert.hpp>
#include <sfz/gl/OpenGL.hpp>
#include <sfz/math/MathHelpers.hpp>

namespace gl {

// Statics
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

static const char* SSAO_FRAGMENT_SHADER = R"(
	#version 330

	// SSAO implementation using normal oriented hemisphere, inspired by this tutorial:
	// http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html

	// Input
	in vec2 uvCoord;
	in vec3 nonNormRayDir;

	// Output
	out vec4 occlusionOut;

	// Constants
	const int MAX_KERNEL_SIZE = 128;

	// Uniforms
	uniform float uFarPlaneDist;
	uniform sampler2D uLinearDepthTexture;
	uniform sampler2D uNormalTexture;
	
	uniform int uKernelSize;
	uniform vec3 uKernel[MAX_KERNEL_SIZE];
	uniform mat4 uProjMatrix;

	uniform vec3 uNoise[16];
	uniform vec2 uDimensions;

	uniform float uRadius;
	uniform float uMinRadius;
	uniform float uOcclusionPower;

	float sampleLinearDepth(vec3 vsPos)
	{
		vec4 offset = uProjMatrix * vec4(vsPos, 1.0);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + vec2(0.5);
		return texture(uLinearDepthTexture, offset.xy).r;
	}

	void main()
	{
		float linDepth = texture(uLinearDepthTexture, uvCoord).r;
		vec3 vsPos = uFarPlaneDist * linDepth * nonNormRayDir / abs(nonNormRayDir.z);
		vec3 normal = texture(uNormalTexture, uvCoord).xyz;

		// Sample 4x4 noise
		ivec2 noiseCoord = ivec2(uvCoord * uDimensions) % ivec2(4);
		vec3 noiseVec = uNoise[noiseCoord.y * 4 + noiseCoord.x];

		// Calculates matrix to rotate kernel into normal hemisphere using Gram Schmidt process
		vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 kernelRot = mat3(tangent, bitangent, normal);

		float minDepthRadius = uMinRadius / uFarPlaneDist;
		float depthRadius = uRadius / uFarPlaneDist;

		float occlusion = 0.0;
		for (int i = 0; i < uKernelSize; i++) {
			vec3 samplePos = vsPos + uRadius * (kernelRot * uKernel[i]);
			float sampleDepth = sampleLinearDepth(samplePos);

			float dist = abs(linDepth - sampleDepth);
			float rangeCheck = (minDepthRadius < dist && dist < depthRadius) ? 1.0 : 0.0;
			//float rangeCheck = smoothstep(0.0, 1.0, depthRadius / abs(linDepth - sampleDepth));
			occlusion += (sampleDepth < linDepth ? 1.0 : 0.0) * rangeCheck; 
		}
		occlusion = pow(1.0 - (occlusion / uKernelSize), uOcclusionPower);

		occlusionOut = vec4(vec3(occlusion), 1.0);
	}
)";

static const char* HORIZONTAL_BLUR_4_SHADER = R"(
	#version 330

	in vec2 uvCoord;
	out vec4 outFragColor;

	uniform sampler2D uTexture;
	uniform float uTexelWidth;

	void main()
	{
		vec2 sampleCoord0 = uvCoord + vec2(uTexelWidth * -1.0, 0.0);
		vec2 sampleCoord1 = uvCoord + vec2(uTexelWidth * 0.5, 0.0);

		vec4 result = vec4(0);
		result += texture(uTexture, sampleCoord0);
		result += texture(uTexture, sampleCoord1);
		result *= 0.5;

		outFragColor = result;
	}
)";

static const char* VERTICAL_BLUR_4_SHADER = R"(
	#version 330

	in vec2 uvCoord;
	out vec4 outFragColor;

	uniform sampler2D uTexture;
	uniform float uTexelHeight;

	void main()
	{
		vec2 sampleCoord0 = uvCoord + vec2(0.0, uTexelHeight * -1.0);
		vec2 sampleCoord1 = uvCoord + vec2(0.0, uTexelHeight * 0.5);

		vec4 result = vec4(0);
		result += texture(uTexture, sampleCoord0);
		result += texture(uTexture, sampleCoord1);
		result *= 0.5;

		outFragColor = result;
	}
)";

static vector<vec3> generateKernel(size_t kernelSize) noexcept
{
	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr1{-1.0f, 1.0f};
	std::uniform_real_distribution<float> distr2{0.0f, 1.0f};

	vector<vec3> kernel{kernelSize};
	for (size_t i = 0; i < kernelSize; i++) {
		// Random vector in z+ hemisphere.
		kernel[i] = normalize(vec3{distr1(gen), distr1(gen), distr2(gen)});
		// Scale it so it has length between 0 and 1.
		//kernel[i] *= distr2(gen); // Naive solution
		// More points closer to base
		float scale = (float)i / (float)kernelSize;
		scale = sfz::lerp(0.1f, 1.0f, scale*scale);
		kernel[i] *= scale;

		//std::uniform_real_distribution<float> tmpDistr{0.2f, std::max(float(i)/float(kernelSize), 0.3f)};
		//kernel[i] *= tmpDistr(gen);
	}

	/*std::cout << "Generated SSAO sample kernel (size = " << kernelSize << ") with values: \n";
	for (auto& val : kernel) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;*/

	return std::move(kernel);
}

static vector<vec3> generateNoiseTexture() noexcept
{
	static_assert(sizeof(vec3) == sizeof(float)*3, "vec3 is padded");
	const size_t NOISE_SIZE = 4;
	const size_t NUM_NOISE_VALUES = NOISE_SIZE * NOISE_SIZE;

	
	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr{-1.0f, 1.0f};

	vector<vec3> noise{NUM_NOISE_VALUES};
	for (size_t i = 0; i < NUM_NOISE_VALUES; ++i) {
		noise[i] = normalize(vec3{distr(gen), distr(gen), 0.0f});
	}

	/*std::cout << "Generated SSAO noise with values: \n";
	for (auto& val : noise) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;*/

	return noise;
}

// SSAO: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SSAO::SSAO(vec2i dimensions, size_t numSamples, float radius, float minRadius, float occlusionPower) noexcept
:
	mDim{dimensions},
	mSSAOProgram{Program::postProcessFromSource(SSAO_FRAGMENT_SHADER)},
	mHorizontalBlurProgram{Program::postProcessFromSource(HORIZONTAL_BLUR_4_SHADER)},
	mVerticalBlurProgram{Program::postProcessFromSource(VERTICAL_BLUR_4_SHADER)},
	mKernelSize{numSamples > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : numSamples},
	mKernel(std::move(generateKernel(mKernelSize))),
	mNoise{generateNoiseTexture()},
	mRadius{radius},
	mMinRadius{minRadius},
	mOcclusionPower{occlusionPower}
{
	resizeFramebuffers();
}

// SSAO: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

uint32_t SSAO::calculate(uint32_t linearDepthTex, uint32_t normalTex, const mat4& projMatrix,
                         float farPlaneDist, bool blur) noexcept
{
	// Render occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mSSAOProgram.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mOcclusionFBO.fbo());
	glViewport(0, 0, mOcclusionFBO.width(), mOcclusionFBO.height());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture buffer uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, linearDepthTex);
	gl::setUniform(mSSAOProgram, "uLinearDepthTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTex);
	gl::setUniform(mSSAOProgram, "uNormalTexture", 1);

	// Other uniforms
	gl::setUniform(mSSAOProgram, "uInvProjMatrix", inverse(projMatrix));
	gl::setUniform(mSSAOProgram, "uFarPlaneDist", farPlaneDist);

	gl::setUniform(mSSAOProgram, "uProjMatrix", projMatrix);

	gl::setUniform(mSSAOProgram, "uKernelSize", (int32_t)mKernelSize);
	gl::setUniform(mSSAOProgram, "uKernel", mKernel.data(), mKernelSize);

	gl::setUniform(mSSAOProgram, "uNoise", mNoise.data(), 16);
	gl::setUniform(mSSAOProgram, "uDimensions", vec2{(float)mDim.x, (float)mDim.y});
	

	//gl::setUniform(mSSAOProgram, "uNoiseTexCoordScale", vec2{(float)mDim.x, (float)mDim.y} / 4.0f);
	gl::setUniform(mSSAOProgram, "uRadius", mRadius);
	gl::setUniform(mSSAOProgram, "uMinRadius", mMinRadius);
	gl::setUniform(mSSAOProgram, "uOcclusionPower", mOcclusionPower);

	mPostProcessQuad.render();

	if (blur) {
		// Horizontal blur
		glUseProgram(mHorizontalBlurProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mTempFBO.fbo());
		glViewport(0, 0, mTempFBO.width(), mTempFBO.height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mOcclusionFBO.texture(0));
		gl::setUniform(mHorizontalBlurProgram, "uTexture", 0);
		gl::setUniform(mHorizontalBlurProgram, "uTexelWidth", 1.0f / mDim.x);

		mPostProcessQuad.render();

		// Vertical blur
		glUseProgram(mVerticalBlurProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mOcclusionFBO.fbo());
		glViewport(0, 0, mOcclusionFBO.width(), mOcclusionFBO.height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTempFBO.texture(0));
		gl::setUniform(mVerticalBlurProgram, "uTexture", 0);
		gl::setUniform(mVerticalBlurProgram, "uTexelHeight", 1.0f / mDim.y);

		mPostProcessQuad.render();
	}

	return mOcclusionFBO.texture(0);
}

// SSAO: Setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::dimensions(vec2i dim) noexcept
{
	sfz_assert_debug(dim.x > 0);
	sfz_assert_debug(dim.y > 0);
	mDim = dim;
	resizeFramebuffers();
}

void SSAO::dimensions(int width, int height) noexcept
{
	dimensions(vec2i{width, height});
}

void SSAO::numSamples(size_t numSamples) noexcept
{
	sfz_assert_debug(numSamples <= MAX_KERNEL_SIZE);
	mKernelSize = numSamples;
	mKernel = std::move(generateKernel(mKernelSize));
}

void SSAO::radius(float radius) noexcept
{
	sfz_assert_debug(radius > 0.0f);
	mRadius = radius;
}

void SSAO::minRadius(float minRadius) noexcept
{
	sfz_assert_debug(minRadius >= 0.0f);
	mMinRadius = minRadius;
}

void SSAO::occlusionPower(float occlusionPower) noexcept
{
	sfz_assert_debug(occlusionPower > 0.0f);
	mOcclusionPower = occlusionPower;
}

// SSAO: Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::resizeFramebuffers() noexcept
{
	mOcclusionFBO = FramebufferBuilder{mDim}
	               .addTexture(0, FBTextureFormat::R_F32, FBTextureFiltering::LINEAR)
	               .build();
	mTempFBO = FramebufferBuilder{mDim}
	          .addTexture(0, FBTextureFormat::R_F32, FBTextureFiltering::LINEAR)
	          .build();
}

} // namespace gl
