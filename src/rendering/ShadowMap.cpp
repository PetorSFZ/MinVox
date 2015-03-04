#include "rendering/ShadowMap.hpp"

namespace vox {

ShadowMap::ShadowMap(int resolution, ShadowMapRes depthRes, bool pcf, const sfz::vec4f& borderColor)
:
	mResolution{resolution},
	mHasPCF{pcf}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	// Generates depth texture
	glGenTextures(1, &mDepthTexture);
	glBindTexture(GL_TEXTURE_2D, mDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(depthRes), resolution, resolution, 0,
	             GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	// Set shadowmap texture min & mag filters (enable/disable pcf)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pcf ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pcf ? GL_LINEAR : GL_NEAREST);

	// Set texture wrap mode to CLAMP_TO_BORDER and set border color.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.glPtr());

	// Magic lines that enable hardware shadowmaps somehow (becomes sampler2Dshadow?)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	// Bind texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);
	glDrawBuffer(GL_NONE); // No color buffer
	glReadBuffer(GL_NONE);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
	glDeleteTextures(1, &mDepthTexture);
	glDeleteFramebuffers(1, &mFBO);
}

} // namespace vox
