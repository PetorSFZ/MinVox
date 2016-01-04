#pragma once
#ifndef VOX_RENDERING_SSAO_HPP
#define VOX_RENDERING_SSAO_HPP

#include <algorithm> // std::swap
#include <random>
#include <vector>
#include <cstddef> // size_t
#include <sfz/GL.hpp>
#include <sfz/Math.hpp>

#include <sfz/gl/PostProcessQuad.hpp>

#include <sfz/gl/OpenGL.hpp>

namespace vox {

using sfz::vec2;
using sfz::vec3;
using sfz::mat4;
using std::vector;
using std::size_t;

// Occlusion Framebuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct OcclusionFramebuffer final {
	GLuint mFBO, mTexture;
	int mWidth, mHeight;

	OcclusionFramebuffer() = delete;
	OcclusionFramebuffer(const OcclusionFramebuffer&) = delete;
	OcclusionFramebuffer& operator= (const OcclusionFramebuffer&) = delete;

	OcclusionFramebuffer(int width, int height) noexcept;
	OcclusionFramebuffer(OcclusionFramebuffer&& other) noexcept;
	OcclusionFramebuffer& operator= (OcclusionFramebuffer&& other) noexcept;
	~OcclusionFramebuffer() noexcept;
};

// SSAO
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class SSAO final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SSAO() = delete;
	SSAO(const SSAO&) = delete;
	SSAO& operator= (const SSAO&) = delete;
	
	SSAO(int width, int height, size_t numSamples, float radius, float occlusionExp) noexcept;
	~SSAO() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint calculate(GLuint posTex, GLuint normalTex, const mat4& projMatrix, float farPlaneDist,
	                 bool clean = false) noexcept;

	// Getters / setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline int textureWidth() const noexcept { return mWidth; }
	inline int textureHeight() const noexcept { return mHeight; }
	void textureSize(int width, int height) noexcept;

	inline size_t numSamples() const noexcept { return mKernelSize; }
	void numSamples(size_t numSamples) noexcept;

	inline float radius() const noexcept { return mRadius; }
	void radius(float radius) noexcept;

	inline float occlusionExp() const noexcept { return mOcclusionExp; }
	void occlusionExp(float occlusionExp) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
private:
	int mWidth, mHeight;

	gl::Program mSSAOProgram, mBlurProgram;
	gl::PostProcessQuad mPostProcessQuad;
	OcclusionFramebuffer mOcclusionFBO, mBlurredFBO;

	static const size_t MAX_KERNEL_SIZE = 256;
	size_t mKernelSize;
	vector<vec3> mKernel;

	static const size_t MAX_NOISE_TEX_WIDTH = 64;
	size_t mNoiseTexWidth;
	GLuint mNoiseTexture;

	float mRadius, mOcclusionExp;
};

} // namespace vox

#endif