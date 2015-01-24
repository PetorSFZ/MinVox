#include "rendering/Assets.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const std::string& basePath()
{
	static const std::string BASE_PATH{SDL_GetBasePath()};
	return BASE_PATH;
}

const std::string& assetsPath()
{
	static const std::string ASSETS_PATH{basePath() + "assets/"};
	return ASSETS_PATH;
}

const std::string& cubeFacePath()
{
	static const std::string CUBE_FACE_128_PATH{assetsPath() + "cube_faces_128pix/"};
	return CUBE_FACE_128_PATH;
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Assets::Assets()
:
	GRASS_FACE{cubeFacePath() + "grass.png"},
	STONE_FACE{cubeFacePath() + "stone.png"}
{
	// Textures loaded and initialized.
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint Assets::getCubeFaceTexture(Voxel voxel) const
{
	switch (voxel.type()) {
	case VoxelType::AIR:
		std::cerr << "AIR shouldn't be rendered." << std::endl;
		std::terminate();
	case VoxelType::LIGHT:
	case VoxelType::MIRROR:
		std::cerr << "Texture not created." << std::endl;
		std::terminate();
	case VoxelType::GRASS: return GRASS_FACE.mHandle;
	case VoxelType::STONE: return STONE_FACE.mHandle;
	case VoxelType::METAL:
	case VoxelType::SAND:
	case VoxelType::GLASS:
		std::cerr << "Texture not created." << std::endl;
		std::terminate();
	}
}

} // namespace vox