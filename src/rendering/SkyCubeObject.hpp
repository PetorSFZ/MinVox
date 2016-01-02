#pragma once
#ifndef VOX_SKY_CUBE_OBJECT_HPP
#define VOX_SKY_CUBE_OBJECT_HPP

#include <sfz/GL.hpp>
#include <sfz/gl/OpenGL.hpp>

namespace vox {

class SkyCubeObject final {
public:
	SkyCubeObject(const SkyCubeObject&) = delete;
	SkyCubeObject& operator= (const SkyCubeObject&) = delete;

	SkyCubeObject() noexcept;
	~SkyCubeObject() noexcept;

	void render() noexcept;

private:
	GLuint vertexArrayObject;
	GLuint posBuffer, normalBuffer, uvBuffer, indexBuffer;
};


} // namespace vox
#endif