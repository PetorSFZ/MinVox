#include "rendering/Assets.hpp"

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

Assets::Assets()
:
	BLUE{cubeFacePath() + "blue_b.png"},
	GREEN{cubeFacePath() + "green_b.png"},
	ORANGE{cubeFacePath() + "orange_b.png"},
	VANILLA{cubeFacePath() + "vanilla_b.png"},
	YELLOW{cubeFacePath() + "yellow_b.png"}
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

} // namespace vox