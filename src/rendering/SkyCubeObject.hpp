#pragma once
#ifndef VOX_SKY_CUBE_OBJECT_HPP
#define VOX_SKY_CUBE_OBJECT_HPP

#include "sfz/GL.hpp"

#include <sfz/MSVC12HackON.hpp>

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
	GLuint posBuffer, uvBuffer, normalBuffer, indexBuffer;
};


} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif