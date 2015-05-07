#include "GlobalConfig.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

GlobalConfig::GlobalConfig() noexcept
{
	mVSync = false;
	mPrintFPS = true;
	mWindowResolutionX = 1000;
	mWindowResolutionY = 800;
	mRetinaAware = false;

	mSSAONumSamples = 16;
	mSSAORadius = 3.0f;
	mSSAOExp = 1.5f;

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