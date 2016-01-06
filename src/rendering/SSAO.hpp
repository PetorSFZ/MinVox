#pragma once
#ifndef SFZ_GL_SSAO_HPP
#define SFZ_GL_SSAO_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include <sfz/gl/Framebuffer.hpp>
#include <sfz/gl/PostProcessQuad.hpp>
#include <sfz/gl/Program.hpp>
#include <sfz/math/Matrix.hpp>
#include <sfz/math/Vector.hpp>

namespace gl {

using sfz::vec2;
using sfz::vec3;
using sfz::vec2i;
using sfz::mat4;

using std::size_t;
using std::uint32_t;
using std::vector;

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

	uint32_t calculate(uint32_t linearDepthTex, uint32_t normalTex, const mat4& projMatrix,
	                   float farPlaneDist, bool blur = false) noexcept;

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

	Program mSSAOProgram, mBlurProgram;
	PostProcessQuad mPostProcessQuad;
	Framebuffer mOcclusionFBO, mBlurredFBO;

	static const size_t MAX_KERNEL_SIZE = 256;
	size_t mKernelSize;
	vector<vec3> mKernel;

	static const size_t MAX_NOISE_TEX_WIDTH = 64;
	size_t mNoiseTexWidth;
	uint32_t mNoiseTexture;

	float mRadius, mOcclusionExp;
};

} // namespace gl
#endif
