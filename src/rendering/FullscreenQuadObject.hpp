#pragma once
#ifndef VOX_FULLSCREEN_QUAD_OBJECT_HPP
#define VOX_FULLSCREEN_QUAD_OBJECT_HPP

#include "sfz/GL.hpp"

namespace vox {

struct FullscreenQuadObject final {
	GLuint mVertexArrayObject;
	GLuint mPosBuffer, mUVBuffer, mIndexBuffer;

	FullscreenQuadObject(const FullscreenQuadObject&) = delete;
	FullscreenQuadObject& operator= (const FullscreenQuadObject&) = delete;

	FullscreenQuadObject();
	~FullscreenQuadObject();
	void render();
};

} // namespace vox

#endif