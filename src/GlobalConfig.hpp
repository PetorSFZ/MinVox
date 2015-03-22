#pragma once
#ifndef VOX_GLOBAL_CONFIG_HPP
#define VOX_GLOBAL_CONFIG_HPP

#include <sfz/MSVC12HackON.hpp>

namespace vox {

struct GlobalConfig {
	bool mVsync, mPrintFPS;

	GlobalConfig() noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif