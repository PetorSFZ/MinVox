#include "rendering/Assets.hpp"

#include <sfz/math/Vector.hpp>
#include <sfz/Assert.hpp>
#include "io/IOUtils.hpp"
#include <iostream>
#include <exception>
#include <new> // std::nothrow

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec2f;

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const string& cubeFacePath()
{
	static const string CUBE_FACE_128_PATH{assetsPath() + "cube_faces_128pix/"};
	return CUBE_FACE_128_PATH;
}

} // anonymous namespace


// Assets: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint Assets::cubeFaceDiffuseTexture() const noexcept
{
	return CUBE_FACE_ATLAS.texture();
}

const TextureRegion& Assets::cubeFaceRegion(Voxel voxel) const noexcept
{
	sfz_assert_debug(voxel.mType < mNumVoxelTypes);
	//sfz_assert_debug(voxel.mType != VOXEL_AIR);
	//sfz_assert_debug(voxel.mType != VOXEL_LIGHT);
	return mCubeFaceRegions[voxel.mType];
}

const string& Assets::cubeFaceName(Voxel voxel) const noexcept
{
	sfz_assert_debug(voxel.mType < mNumVoxelTypes);
	return mCubeFaceNames[voxel.mType];
}

GLuint Assets::cubeFaceIndividualTexture(Voxel voxel) const noexcept
{
	switch (voxel.mType) {
	case VOXEL_AIR:
		std::cerr << "AIR shouldn't be rendered." << std::endl;
		std::terminate();
	case VOXEL_BLUE: return BLUE.mHandle;
	case VOXEL_GREEN: return GREEN.mHandle;
	case VOXEL_ORANGE: return ORANGE.mHandle;
	case VOXEL_VANILLA: return VANILLA.mHandle;
	case VOXEL_YELLOW: return YELLOW.mHandle;
	default:
		std::cerr << "Texture not created." << std::endl;
		std::terminate();
	}
}

// Assets: Private constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Assets& Assets::INSTANCE() noexcept
{
	static Assets assets;
	return assets;
}

Assets::Assets() noexcept
:
	mSpriteBatch{3000},
	mFontRenderer{assetsPath() + "fonts/SourceCodePro-Regular.ttf", 1024, 1024, 74.0f, 3000},

	mNumVoxelTypes{7},
	CUBE_FACE_ATLAS{cubeFacePath(), {"blue_b.png", "green_b.png", "orange_b.png", "vanilla_b.png",
	                                 "yellow_b.png"}},

	mCubeFaceRegions{new (std::nothrow) TextureRegion[mNumVoxelTypes]},
	mCubeFaceNames{new (std::nothrow) string[mNumVoxelTypes]},

	BLUE{cubeFacePath() + "blue_b.png"},
	GREEN{cubeFacePath() + "green_b.png"},
	ORANGE{cubeFacePath() + "orange_b.png"},
	VANILLA{cubeFacePath() + "vanilla_b.png"},
	YELLOW{cubeFacePath() + "yellow_b.png"}
{
	mCubeFaceRegions[VOXEL_AIR] = TextureRegion{vec2f{0.0f, 0.0f}, vec2f{0.0f, 0.0f}}; // 0
	mCubeFaceRegions[VOXEL_LIGHT] = TextureRegion{vec2f{0.0f, 0.0f}, vec2f{0.0f, 0.0f}}; // 1
	mCubeFaceRegions[VOXEL_BLUE] = *CUBE_FACE_ATLAS.textureRegion("blue_b.png");
	mCubeFaceRegions[VOXEL_GREEN] = *CUBE_FACE_ATLAS.textureRegion("green_b.png");
	mCubeFaceRegions[VOXEL_ORANGE] = *CUBE_FACE_ATLAS.textureRegion("orange_b.png");
	mCubeFaceRegions[VOXEL_VANILLA] = *CUBE_FACE_ATLAS.textureRegion("vanilla_b.png");
	mCubeFaceRegions[VOXEL_YELLOW] = *CUBE_FACE_ATLAS.textureRegion("yellow_b.png");

	mCubeFaceNames[VOXEL_AIR] = "AIR";
	mCubeFaceNames[VOXEL_LIGHT] = "LIGHT";
	mCubeFaceNames[VOXEL_BLUE] = "BLUE";
	mCubeFaceNames[VOXEL_GREEN] = "GREEN";
	mCubeFaceNames[VOXEL_ORANGE] = "ORANGE";
	mCubeFaceNames[VOXEL_VANILLA] = "VANILLA";
	mCubeFaceNames[VOXEL_YELLOW] = "YELLOW";
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>