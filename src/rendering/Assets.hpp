#pragma once
#ifndef VOX_RENDERING_ASSETS_HPP
#define VOX_RENDERING_ASSETS_HPP

#include "sfz/GL.hpp"
#include "model/Voxel.hpp"
#include <memory>
#include <cstddef> // size_t
#include <string>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::unique_ptr;
using std::size_t;
using std::string;

using gl::Texture;
using gl::TextureRegion;
using gl::TexturePacker;
using gl::SpriteBatch;
using gl::FontRenderer;

// Assets
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class Assets final {
public:
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SpriteBatch mSpriteBatch;
	FontRenderer mFontRenderer;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	static Assets& INSTANCE() noexcept;

	inline size_t numVoxelTypes() const noexcept { return mNumVoxelTypes; }
	GLuint cubeFaceDiffuseTexture() const noexcept;
	const TextureRegion& cubeFaceRegion(Voxel voxel) const noexcept;
	const string& cubeFaceName(Voxel voxel) const noexcept;

	GLuint cubeFaceIndividualTexture(Voxel voxel) const noexcept; // LEGACY

private:
	// Private constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Assets() noexcept;
	Assets(const Assets&) = delete;
	Assets& operator= (const Assets&) = delete;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	const size_t mNumVoxelTypes;
	TexturePacker CUBE_FACE_ATLAS;
	unique_ptr<TextureRegion[]> mCubeFaceRegions;
	unique_ptr<string[]> mCubeFaceNames;

	Texture BLUE,
	        GREEN,
	        ORANGE,
	        VANILLA,
			YELLOW;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>

#endif