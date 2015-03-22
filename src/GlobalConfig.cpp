#include "GlobalConfig.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

GlobalConfig::GlobalConfig() noexcept
{
	mVsync = false;
	mPrintFPS = false;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>