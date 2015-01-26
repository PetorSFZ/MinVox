#include "io/IOUtils.hpp"

namespace vox {

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


} // namespace vox