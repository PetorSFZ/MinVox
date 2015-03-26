#pragma once
#ifndef VOX_GLOBAL_CONFIG_HPP
#define VOX_GLOBAL_CONFIG_HPP

#include <cstddef> // size_t

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::size_t;

struct GlobalConfig {
	bool mVsync, mPrintFPS;

	size_t mVerticalRange, mHorizontalRange;

	GlobalConfig() noexcept;
};

/** @brief Gets the global config instance. */
GlobalConfig& getGlobalConfig() noexcept;

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif