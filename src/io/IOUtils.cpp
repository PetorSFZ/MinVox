#include "io/IOUtils.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <direct.h>
#elif __APPLE__
#include <sys/stat.h>
#elif __unix
#include <sys/stat.h>
#endif

namespace vox {

namespace {

#ifdef _WIN32
bool existsDir(const std::string& path)
{
	DWORD ftyp = GetFileAttributesA(path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
	return false;
}
#endif

} // anonymous namespace

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

bool exists(const std::string& path)
{
#ifdef _WIN32
	std::FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) {
		return false || existsDir(path);
	} else {
		fclose(file);
		return true || existsDir(path);
	}
#else
	std::FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) {
		return false;
	} else {
		fclose(file);
		return true;
	}
#endif
}

bool createDirectory(const std::string& path)
{
#ifdef _WIN32
	int res = _mkdir(path.c_str());
	if (res == 0) return true;
	return false;
#elif __APPLE__
	int res = mkdir(path.c_str(), 0775);
	if (res == 0) return true;
	else return false;
#elif __unix
	int res = mkdir(path.c_str(), 0775);
	if (res == 0) return true;
	else return false;
#else
	std::cerr << "Unsupported platform" << std::endl;
	std::terminate();
#endif
}

} // namespace vox