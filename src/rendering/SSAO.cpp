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

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 uvCoord;
	in vec3 nonNormRayDir;

	// Output
	out vec4 occlusionOut;

	// Constants
	const int MAX_KERNEL_SIZE = 256;

	// Uniforms
	uniform float uFarPlaneDist;
	uniform sampler2D uLinearDepthTexture;
	uniform sampler2D uNormalTexture;
	uniform sampler2D uNoiseTexture;
		
	uniform int uKernelSize;
	uniform vec3 uKernel[MAX_KERNEL_SIZE];
	uniform mat4 uProjectionMatrix;

	uniform vec2 uNoiseTexCoordScale;
	uniform float uRadius;
	uniform float uOcclusionExp;

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

		float linDepth = texture(uLinearDepthTexture, uvCoord).r;
		vec3 vsPos = uFarPlaneDist * linDepth * nonNormRayDir / abs(nonNormRayDir.z);
		vec3 normal = normalize(texture(uNormalTexture, uvCoord).xyz);
		vec3 noiseVec = texture(uNoiseTexture, uvCoord * uNoiseTexCoordScale).xyz;

		// Calculates matrix to rotate kernel into normal hemisphere using Gram Schmidt process
		vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
		vec3 bitangent = cross(tangent, normal);
		mat3 kernelRot = mat3(tangent, bitangent, normal);

		float radius = uRadius / uFarPlaneDist;

		float occlusion = 0.0;
		for (int i = 0; i < uKernelSize; i++) {
			vec3 samplePos = vsPos + uRadius * (kernelRot * uKernel[i]);
			vec2 sampleTexCoord = texCoordFromVSPos(samplePos);
			float sampleDepth = texture(uLinearDepthTexture, sampleTexCoord).r;

			float rangeCheck = abs(linDepth - sampleDepth) < radius ? 1.0 : 0.0;
			occlusion += (sampleDepth >= linDepth ? 0.0 : 1.0) * rangeCheck;

			//float rangeCheck = abs(linDepth - sampleDepth) < uRadius ? 1.0 : 0.0;
			//occlusion += (sampleDepth <= samplePos.z ? 1.0 : 0.0) * rangeCheck;
			
			//float rangeCheck = smoothstep(0.0, 1.0, uRadius / abs(linDepth - sampleDepth));
			//occlusion += (step(sampleDepth, samplePos.z)); // * rangeCheck);
		}
		occlusion = 1.0 - (occlusion / uKernelSize);
		occlusion = pow(occlusion, uOcclusionExp);

		occlusionOut = vec4(occlusion, 0.0, 0.0, 1.0);
	}
)";

static const char* SSAO_BLUR_FRAGMENT_SHADER = R"(
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

static vector<vec3> generateKernel(size_t kernelSize) noexcept
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

static GLuint generateNoiseTexture(size_t noiseTexWidth) noexcept
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

// SSAO: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SSAO::SSAO(vec2i dimensions, size_t numSamples, float radius, float occlusionExp) noexcept
:
	mDim{dimensions},
	mSSAOProgram{Program::postProcessFromSource(SSAO_FRAGMENT_SHADER)},
	mBlurProgram{Program::postProcessFromSource(SSAO_BLUR_FRAGMENT_SHADER)},
	mKernelSize{numSamples > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : numSamples},
	mKernel(std::move(generateKernel(MAX_KERNEL_SIZE))),
	mNoiseTexWidth{4},
	mNoiseTexture{generateNoiseTexture(mNoiseTexWidth)},
	mRadius{radius},
	mOcclusionExp{occlusionExp}
{
	resizeFramebuffers();
}

SSAO::~SSAO() noexcept
{
	glDeleteTextures(1, &mNoiseTexture);
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

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);
	gl::setUniform(mSSAOProgram, "uNoiseTexture", 2);

	// Other uniforms
	gl::setUniform(mSSAOProgram, "uInvProjMatrix", inverse(projMatrix));
	gl::setUniform(mSSAOProgram, "uFarPlaneDist", farPlaneDist);

	gl::setUniform(mSSAOProgram, "uProjectionMatrix", projMatrix);

	gl::setUniform(mSSAOProgram, "uKernelSize", static_cast<int>(mKernelSize));
	gl::setUniform(mSSAOProgram, "uKernel", static_cast<vec3*>(mKernel.data()), mKernelSize);

	gl::setUniform(mSSAOProgram, "uNoiseTexCoordScale",
	     vec2{(float)mDim.x, (float)mDim.y} / (float)mNoiseTexWidth);
	gl::setUniform(mSSAOProgram, "uRadius", mRadius);
	gl::setUniform(mSSAOProgram, "uOcclusionExp", mOcclusionExp);

	mPostProcessQuad.render();

	// Blur occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	if (blur) {
		glUseProgram(mBlurProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mBlurredFBO.fbo());
		glViewport(0, 0, mBlurredFBO.width(), mBlurredFBO.height());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mOcclusionFBO.texture(0));
		gl::setUniform(mBlurProgram, "uOcclusionTexture", 0);

		mPostProcessQuad.render();

		return mBlurredFBO.texture(0);
	} else {
		return mOcclusionFBO.texture(0);
	}
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
}

void SSAO::radius(float radius) noexcept
{
	sfz_assert_debug(radius > 0.0f);
	mRadius = radius;
}

void SSAO::occlusionExp(float occlusionExp) noexcept
{
	sfz_assert_debug(occlusionExp > 0.0f);
	mOcclusionExp = occlusionExp;
}

// SSAO: Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::resizeFramebuffers() noexcept
{
	mOcclusionFBO = FramebufferBuilder{mDim}
	               .addTexture(0, FBTextureFormat::R_F32, FBTextureFiltering::LINEAR)
	               .build();
	mBlurredFBO = FramebufferBuilder{mDim}
	             .addTexture(0, FBTextureFormat::R_F32, FBTextureFiltering::LINEAR)
	             .build();
}

} // namespace gl
