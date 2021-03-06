#pragma once

#include <sfz/gl/Framebuffer.hpp>
#include <sfz/gl/PostProcessQuad.hpp>
#include <sfz/gl/Program.hpp>
#include <sfz/math/Vector.hpp>

namespace gl {

using sfz::vec2i;

class SMAA final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SMAA() = default;
	SMAA(const SMAA&) = delete;
	SMAA& operator= (const SMAA&) = delete;
	
	SMAA(vec2i dimensions) noexcept;
	SMAA(SMAA&& other) noexcept;
	SMAA& operator= (SMAA&& other);
	~SMAA() noexcept;

	// Public method
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	uint32_t apply(uint32_t tex) noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec2i mDimensions;
	PostProcessQuad mPostProcessQuad;
	Program mSMAAEdgeDetection, mSMAAWeightCalculation, mSMAABlending;
	Framebuffer mEdgesFB, mWeightsFB, mResultFB;

	uint32_t mAreaTex = 0;
	uint32_t mSearchTex = 0;
};

} // namespace gl
