#pragma once
#ifndef VOX_SHADOW_MAP_HPP
#define VOX_SHADOW_MAP_HPP

#include "sfz/GL.hpp"
#include <sfz/math/Vector.hpp>

namespace vox {

/** Enum used to set the amount of bits per depth value in a ShadowMap. */
enum class ShadowMapRes : GLint {
	BITS_16 = GL_DEPTH_COMPONENT16,
	BITS_24 = GL_DEPTH_COMPONENT24,
	BITS_32 = GL_DEPTH_COMPONENT32
};

/** An OpenGL ShadowMap FBO. */
struct ShadowMap {
	GLuint mFBO, mDepthTexture;
	int mResolution;
	bool mHasPCF;

	ShadowMap() = delete;
	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator= (const ShadowMap&) = delete;

	ShadowMap(int resolution, ShadowMapRes depthRes, bool pcf, const sfz::vec4& borderColor);
	~ShadowMap();
};

} // namespace vox

#endif