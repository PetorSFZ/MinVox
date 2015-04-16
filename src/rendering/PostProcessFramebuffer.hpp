#pragma once
#ifndef VOX_RENDERING_POST_PROCESS_FRAMEBUFFER_HPP
#define VOX_RENDERING_POST_PROCESS_FRAMEBUFFER_HPP

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

struct PostProcessFramebuffer final {
	GLuint mFBO, mTexture;
	int mWidth, mHeight;

	PostProcessFramebuffer() = delete;
	PostProcessFramebuffer(const PostProcessFramebuffer&) = delete;
	PostProcessFramebuffer& operator= (const PostProcessFramebuffer&) = delete;

	PostProcessFramebuffer(int width, int height) noexcept;
	PostProcessFramebuffer(PostProcessFramebuffer&& other) noexcept;
	PostProcessFramebuffer& operator= (PostProcessFramebuffer&& other) noexcept;
	~PostProcessFramebuffer() noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif