#pragma once
#ifndef VOX_RENDERING_ASSETS_HPP
#define VOX_RENDERING_ASSETS_HPP

#include <iostream>
#include <string>
#include <exception>
#include "sfz/GL.hpp"

#include "model/Voxel.hpp"
#include "io/IOUtils.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using gl::Texture;
using gl::TextureRegion;
using gl::TexturePacker;
using gl::SpriteBatch;


struct Assets final {

	Texture BLUE,
	        GREEN,
	        ORANGE,
	        VANILLA,
			YELLOW;

	TexturePacker CUBE_FACE_ATLAS;
	const TextureRegion BLUE_TR,
	                    GREEN_TR,
						ORANGE_TR,
						VANILLA_TR,
						YELLOW_TR;


	SpriteBatch mSpriteBatch;

	Assets();
	Assets(const Assets&) = delete;
	Assets& operator= (const Assets&) = delete;

	GLuint getCubeFaceTexture(Voxel voxel) const noexcept;
	const TextureRegion& getCubeFaceTextureRegion(Voxel voxel) const noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>

#endif