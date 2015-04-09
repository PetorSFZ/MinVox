#pragma once
#ifndef VOX_RENDERING_SSAO_HPP
#define VOX_RENDERING_SSAO_HPP

#include <random>
#include <vector>
#include <cstddef> // size_t
#include <sfz/GL.hpp>
#include <sfz/Math.hpp>

#include "rendering/FullscreenQuadObject.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3f;
using sfz::mat4f;
using std::vector;
using std::size_t;

class SSAO {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SSAO() = delete;
	SSAO(const SSAO&) = delete;
	SSAO& operator= (const SSAO&) = delete;
	
	SSAO(size_t kernelSize) noexcept;
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

	static const size_t MAX_KERNEL_SIZE = 128;
	size_t mKernelSize;
	vector<vec3f> mKernel;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif