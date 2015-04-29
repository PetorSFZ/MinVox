#include "rendering/GBuffer.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// GBuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GBuffer::GBuffer(int width, int height) noexcept
:
	mWidth{width},
	mHeight{height}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	// Depth buffer
	glGenTextures(1, &mDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, mDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthBuffer, 0);

	// Diffuse texture
	glGenTextures(1, &mDiffuseTexture);
	glBindTexture(GL_TEXTURE_2D, mDiffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mDiffuseTexture, 0);

	// Position texture
	glGenTextures(1, &mPositionTexture);
	glBindTexture(GL_TEXTURE_2D, mPositionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPositionTexture, 0);

	// Normal texture
	glGenTextures(1, &mNormalTexture);
	glBindTexture(GL_TEXTURE_2D, mNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mNormalTexture, 0);


	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, drawBuffers);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBuffer::GBuffer(GBuffer&& other) noexcept
{
	glGenFramebuffers(1, &mFBO);
	glGenTextures(1, &mDepthBuffer);
	glGenTextures(1, &mDiffuseTexture);
	glGenTextures(1, &mPositionTexture);
	glGenTextures(1, &mNormalTexture);

	std::swap(mFBO, other.mFBO);
	std::swap(mDepthBuffer, other.mDepthBuffer);
	std::swap(mDiffuseTexture, other.mDiffuseTexture);
	std::swap(mPositionTexture, other.mPositionTexture);
	std::swap(mNormalTexture, other.mNormalTexture);

	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
}

GBuffer& GBuffer::operator= (GBuffer&& other) noexcept
{
	std::swap(mFBO, other.mFBO);
	std::swap(mDepthBuffer, other.mDepthBuffer);
	std::swap(mDiffuseTexture, other.mDiffuseTexture);
	std::swap(mPositionTexture, other.mPositionTexture);
	std::swap(mNormalTexture, other.mNormalTexture);

	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);

	return *this;
}

GBuffer::~GBuffer() noexcept
{
	glDeleteTextures(1, &mDepthBuffer);
	glDeleteTextures(1, &mDiffuseTexture);
	glDeleteTextures(1, &mPositionTexture);
	glDeleteTextures(1, &mNormalTexture);
	glDeleteFramebuffers(1, &mFBO);
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>