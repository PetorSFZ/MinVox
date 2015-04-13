#pragma once
#ifndef VOX_RENDERING_SSAO_HPP
#define VOX_RENDERING_SSAO_HPP

#include <algorithm> // std::swap
#include <random>
#include <vector>
#include <cstddef> // size_t
#include <sfz/GL.hpp>
#include <sfz/Math.hpp>

#include "rendering/FullscreenQuadObject.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec2f;
using sfz::vec3f;
using sfz::mat4f;
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
	
	SSAO(int width, int height, size_t numSamples, float radius) noexcept;
	~SSAO() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void apply(GLuint targetFramebuffer,
	           GLuint colorTex, GLuint depthTex, GLuint normalTex, GLuint posTex,
	           const mat4f& projectionMatrix) noexcept;

	void setSize(int width, int height) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
private:
	int mWidth, mHeight;

	GLuint mSSAOProgram, mBlurProgram;
	FullscreenQuadObject mFullscreenQuad;
	OcclusionFramebuffer mOcclusionFBO, mBlurredFBO;

	static const size_t MAX_KERNEL_SIZE = 256;
	size_t mKernelSize;
	vector<vec3f> mKernel;

	static const size_t MAX_NOISE_TEX_WIDTH = 64;
	size_t mNoiseTexWidth;
	GLuint mNoiseTexture;

	float mRadius;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif