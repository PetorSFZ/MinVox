#pragma once
#ifndef VOX_RENDERING_POST_PROCESS_FRAMEBUFFER_HPP
#define VOX_RENDERING_POST_PROCESS_FRAMEBUFFER_HPP

#include <algorithm> // std::swap

#include "sfz/GL.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

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

struct DirectionalLightingFramebuffer final {
	GLuint mFBO, mTexture, mStencilBuffer;
	int mWidth, mHeight;

	DirectionalLightingFramebuffer() = delete;
	DirectionalLightingFramebuffer(const DirectionalLightingFramebuffer&) = delete;
	DirectionalLightingFramebuffer& operator= (const DirectionalLightingFramebuffer&) = delete;

	DirectionalLightingFramebuffer(int width, int height) noexcept;
	DirectionalLightingFramebuffer(DirectionalLightingFramebuffer&& other) noexcept;
	DirectionalLightingFramebuffer& operator= (DirectionalLightingFramebuffer&& other) noexcept;
	~DirectionalLightingFramebuffer() noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif