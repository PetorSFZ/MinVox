#include "GlobalConfig.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

GlobalConfig::GlobalConfig() noexcept
{
	mVSync = false;
	mFullscreen = true;
	mPrintFPS = false;
	mWindowResolutionX = 1000;
	mWindowResolutionY = 800;
	mLockedResolution = true;
	//mLockedResolutionY = 640;
	mLockedResolutionY = 720;
	mRetinaAware = true;

	mShadowResolution = 800;
	mShadowPCF = false;

	mSSAONumSamples = 16;
	mSSAORadius = 2.0f;
	mSSAOExp = 1.1f;

	mLightShaftSamples = 38;
	mLightShaftRange = 18.0f;

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