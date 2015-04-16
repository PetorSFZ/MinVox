#pragma once
#ifndef VOX_G_BUFFER_HPP
#define VOX_G_BUFFER_HPP

#include <algorithm> // std::swap

#include <sfz/Assert.hpp>
#include <sfz/GL.hpp>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

struct GBuffer final {
	GLuint mFBO, mDepthBuffer, mDiffuseTexture, mPositionTexture, mNormalTexture;
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

#include <sfz/MSVC12HackOFF.hpp>
#endif