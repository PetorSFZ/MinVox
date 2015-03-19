#include "rendering/Framebuffer.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// BigFramebuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

BigFramebuffer::BigFramebuffer(int width, int height) noexcept
:
	mWidth{width},
	mHeight{height}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);

	// Color texture
	glGenTextures(1, &mColorTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, mColorTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, mColorTexture, 0);

	// Normal texture
	glGenTextures(1, &mNormalTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, mNormalTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, mNormalTexture, 0);

	// Depth texture
	glGenTextures(1, &mDepthTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, mDepthTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, mDepthTexture, 0);

	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, drawBuffers);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

BigFramebuffer& BigFramebuffer::operator= (BigFramebuffer&& other) noexcept
{
	std::swap(mFrameBufferObject, other.mFrameBufferObject);
	std::swap(mColorTexture, other.mColorTexture);
	std::swap(mNormalTexture, other.mNormalTexture);
	std::swap(mDepthTexture, other.mDepthTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
	return *this;
}

BigFramebuffer::~BigFramebuffer() noexcept
{
	glDeleteTextures(1, &mColorTexture);
	glDeleteTextures(1, &mDepthTexture);
	glDeleteTextures(1, &mNormalTexture);
	glDeleteFramebuffers(1, &mFrameBufferObject);
}

// Framebuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Framebuffer::Framebuffer(int width, int height) noexcept
:
	mWidth{width},
	mHeight{height}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFrameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);

	// Color texture
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, mTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, mTexture, 0);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer& Framebuffer::operator= (Framebuffer&& other) noexcept
{
	std::swap(mFrameBufferObject, other.mFrameBufferObject);
	std::swap(mTexture, other.mTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
	return *this;
}

Framebuffer::~Framebuffer() noexcept
{
	glDeleteTextures(1, &mTexture);
	glDeleteFramebuffers(1, &mFrameBufferObject);
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>