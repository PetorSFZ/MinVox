#include "GlobalConfig.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

GlobalConfig::GlobalConfig() noexcept
{
	mVsync = false;
	mPrintFPS = true;

	mHorizontalRange = 2;
	mVerticalRange = 1;
}

GlobalConfig& getGlobalConfig() noexcept
{
	static GlobalConfig globalConfig;
	return globalConfig;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>