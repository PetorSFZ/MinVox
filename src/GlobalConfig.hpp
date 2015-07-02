#pragma once
#ifndef VOX_GLOBAL_CONFIG_HPP
#define VOX_GLOBAL_CONFIG_HPP

#include <cstddef> // size_t

namespace vox {

using std::size_t;

struct GlobalConfig {
	bool mVSync, mFullscreen, mPrintFPS;
	int mWindowResolutionX, mWindowResolutionY;
	bool mLockedResolution;
	int mLockedResolutionY;
	bool mRetinaAware;

	int mShadowResolution;
	bool mShadowPCF;
	int mLightShaftSamples;
	float mLightShaftRange;

	bool mSSAOClean;
	size_t mSSAONumSamples;
	float mSSAORadius, mSSAOExp;

	size_t mVerticalRange, mHorizontalRange;

	GlobalConfig() noexcept;
};

/** @brief Gets the global config instance. */
GlobalConfig& getGlobalConfig() noexcept;

} // namespace vox

#endif