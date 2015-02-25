#pragma once
#ifndef VOX_RENDERING_CUBE_OBJECT_HPP
#define VOX_RENDERING_CUBE_OBJECT_HPP

#include <sfz/Math.hpp>
#include "sfz/GL.hpp"

namespace vox {

class CubeObject final {
public:
	CubeObject();
	CubeObject(const CubeObject&) = delete;
	CubeObject& operator= (const CubeObject&) = delete;
	~CubeObject();

	void render();

private:
	GLuint vertexArrayObject;
	GLuint posBuffer, uvBuffer, normalBuffer, indexBuffer;
};

} // namespace vox

#endif