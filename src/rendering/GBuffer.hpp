#pragma once
#ifndef VOX_G_BUFFER_HPP
#define VOX_G_BUFFER_HPP

#include <algorithm> // std::swap

#include <sfz/Assert.hpp>
#include <sfz/GL.hpp>



namespace vox {

struct GBuffer final {
	GLuint mFBO, mDepthBuffer, mDiffuseTexture, mPositionTexture, mNormalTexture, mEmissiveTexture,
	       mMaterialTexture;
	int mWidth, mHeight;

	GBuffer() = delete;
	GBuffer(const GBuffer&) = delete;
	GBuffer& operator= (const GBuffer&) = delete;

	GBuffer(int width, int height) noexcept;
	GBuffer(GBuffer&& other) noexcept;
	GBuffer& operator= (GBuffer&& other) noexcept;
	~GBuffer() noexcept;
};

} // namespace vox


#endif