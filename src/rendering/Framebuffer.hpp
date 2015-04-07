#pragma once
#ifndef VOX_RENDERING_FRAMEBUFFER_HPP
#define VOX_RENDERING_FRAMEBUFFER_HPP

#include <algorithm> // std::swap

#include "sfz/GL.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

struct BigFramebuffer final {
	GLuint mFrameBufferObject, mColorTexture, mNormalTexture, mDepthTexture, mPositionTexture;
	int mWidth, mHeight;

	BigFramebuffer() = delete;
	BigFramebuffer(const BigFramebuffer&) = delete;
	BigFramebuffer(BigFramebuffer&&) = delete;
	BigFramebuffer& operator= (const BigFramebuffer&) = delete;

	BigFramebuffer(int width, int height) noexcept;
	BigFramebuffer& operator= (BigFramebuffer&& other) noexcept;
	~BigFramebuffer() noexcept;
};

struct Framebuffer final {
	GLuint mFrameBufferObject, mTexture;
	int mWidth, mHeight;

	Framebuffer() = delete;
	Framebuffer(const Framebuffer&) = delete;
	Framebuffer(Framebuffer&&) = delete;
	Framebuffer& operator= (const Framebuffer&) = delete;

	Framebuffer(int width, int height) noexcept;
	Framebuffer& operator= (Framebuffer&& other) noexcept;
	~Framebuffer() noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif