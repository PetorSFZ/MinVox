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

using std::int32_t;
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
	
	SSAO(vec2i dimensions, size_t numSamples, float radius, float occlusionPower) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	uint32_t calculate(uint32_t linearDepthTex, uint32_t normalTex, const mat4& projMatrix,
	                   float farPlaneDist, bool blur = false) noexcept;

	// Getters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline vec2i dimensions() const noexcept { return mDim; }
	inline int32_t width() const noexcept { return mDim.x; }
	inline int32_t height() const noexcept { return mDim.y; }
	inline size_t numSamples() const noexcept { return mKernelSize; }
	inline float radius() const noexcept { return mRadius; }
	inline float occlusionPower() const noexcept { return mOcclusionPower; }

	// Setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void dimensions(vec2i dim) noexcept;
	void dimensions(int width, int height) noexcept;
	void numSamples(size_t numSamples) noexcept;
	void radius(float radius) noexcept;
	void occlusionPower(float occlusionPower) noexcept;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void resizeFramebuffers() noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec2i mDim;

	Program mSSAOProgram, mHorizontalBlurProgram, mVerticalBlurProgram;
	PostProcessQuad mPostProcessQuad;
	Framebuffer mOcclusionFBO, mTempFBO;

	static const size_t MAX_KERNEL_SIZE = 256;
	size_t mKernelSize;
	vector<vec3> mKernel;

	vector<vec3> mNoise;

	float mRadius, mOcclusionPower;
};

} // namespace gl
#endif
