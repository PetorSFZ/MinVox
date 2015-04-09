#pragma once
#ifndef VOX_RENDERING_SSAO_HPP
#define VOX_RENDERING_SSAO_HPP

#include <sfz/GL.hpp>
#include <sfz/Math.hpp>

#include "rendering/FullscreenQuadObject.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::mat4f;

class SSAO {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SSAO(const SSAO&) = delete;
	SSAO& operator= (const SSAO&) = delete;
	
	SSAO() noexcept;
	~SSAO() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void apply(GLuint targetFramebuffer, int framebufferWidth, int framebufferHeight,
	           GLuint colorTex, GLuint depthTex, GLuint normalTex, GLuint posTex,
	           const mat4f& projectionMatrix) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
private:
	GLuint mSSAOProgram;
	FullscreenQuadObject mFullscreenQuad;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif