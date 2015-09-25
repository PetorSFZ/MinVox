#pragma once
#ifndef VOX_RENDERING_CUBE_OBJECT_HPP
#define VOX_RENDERING_CUBE_OBJECT_HPP

#include <cmath>

#include <sfz/GL.hpp>
#include <sfz/gl/OpenGL.hpp> // TODO: Move to .cpp

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