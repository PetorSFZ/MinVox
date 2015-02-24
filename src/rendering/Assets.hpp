#pragma once
#ifndef VOX_RENDERING_ASSETS_HPP
#define VOX_RENDERING_ASSETS_HPP

#include <iostream>
#include <string>
#include <exception>
#include "sfz/GL.hpp"

#include "model/Voxel.hpp"
#include "io/IOUtils.hpp"

namespace vox {

struct Assets final {

	gl::Texture BLUE,
	            GREEN,
	            ORANGE,
	            VANILLA,
	            YELLOW;

	Assets();
	Assets(const Assets&) = delete;
	Assets& operator= (const Assets&) = delete;

	GLuint getCubeFaceTexture(Voxel voxel) const;
};

} // namespace vox

#endif