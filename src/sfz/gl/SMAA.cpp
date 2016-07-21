#include "sfz/gl/SMAA.hpp"

namespace gl {

// SMAA: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SMAA::SMAA(vec2i dimensions) noexcept
{
	mDimensions = dimensions;

	mSMAAEdgeDetection;
	mSMAAWeightCalculation;
	mSMAABlending;

	mEdgesFB = FramebufferBuilder(dimensions)
	          .addTexture(0, FBTextureFormat::RG_U8, FBTextureFiltering::LINEAR)
	          .build();
	mWeightsFB = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::R_U8, FBTextureFiltering::LINEAR)
	            .build();
	mResultFB  = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR)
	            .build();
}

SMAA::~SMAA() noexcept
{
	
}

// SMAA: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

uint32_t SMAA::apply(uint32_t tex) noexcept
{
	return tex;
}

} // namespace gl
