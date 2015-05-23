#include "rendering/Assets.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const std::string& cubeFacePath()
{
	static const std::string CUBE_FACE_128_PATH{assetsPath() + "cube_faces_128pix/"};
	return CUBE_FACE_128_PATH;
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const Assets& Assets::INSTANCE() noexcept
{
	static Assets assets;
	return assets;
}

Assets::Assets() noexcept
:
	BLUE{cubeFacePath() + "blue_b.png"},
	GREEN{cubeFacePath() + "green_b.png"},
	ORANGE{cubeFacePath() + "orange_b.png"},
	VANILLA{cubeFacePath() + "vanilla_b.png"},
	YELLOW{cubeFacePath() + "yellow_b.png"},

	CUBE_FACE_ATLAS{cubeFacePath(), {"blue_b.png", "green_b.png", "orange_b.png", "vanilla_b.png",
	                                 "yellow_b.png"}},

	BLUE_TR{*CUBE_FACE_ATLAS.textureRegion("blue_b.png")},
	GREEN_TR{*CUBE_FACE_ATLAS.textureRegion("green_b.png")},
	ORANGE_TR{*CUBE_FACE_ATLAS.textureRegion("orange_b.png")},
	VANILLA_TR{*CUBE_FACE_ATLAS.textureRegion("vanilla_b.png")},
	YELLOW_TR{*CUBE_FACE_ATLAS.textureRegion("yellow_b.png")},

	mSpriteBatch{1000}
{
	// Textures loaded and initialized.
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint Assets::getCubeFaceTexture(Voxel voxel) const noexcept
{
	switch (voxel.type()) {
	case VoxelType::AIR:
		std::cerr << "AIR shouldn't be rendered." << std::endl;
		std::terminate();
	case VoxelType::BLUE: return BLUE.mHandle;
	case VoxelType::GREEN: return GREEN.mHandle;
	case VoxelType::ORANGE: return ORANGE.mHandle;
	case VoxelType::VANILLA: return VANILLA.mHandle;
	case VoxelType::YELLOW: return YELLOW.mHandle;
	default:
		std::cerr << "Texture not created." << std::endl;
		std::terminate();
	}
}

const TextureRegion& Assets::getCubeFaceTextureRegion(Voxel voxel) const noexcept
{
	switch (voxel.type()) {
	case VoxelType::AIR:
		std::cerr << "AIR shouldn't be rendered." << std::endl;
		std::terminate();
	case VoxelType::BLUE: return BLUE_TR;
	case VoxelType::GREEN: return GREEN_TR;
	case VoxelType::ORANGE: return ORANGE_TR;
	case VoxelType::VANILLA: return VANILLA_TR;
	case VoxelType::YELLOW: return YELLOW_TR;
	default:
		std::cerr << "TextureRegion doesn't exist." << std::endl;
		std::terminate();
	}
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>