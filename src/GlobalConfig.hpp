#pragma once
#ifndef VOX_GLOBAL_CONFIG_HPP
#define VOX_GLOBAL_CONFIG_HPP

#include <cstdint>

#include <sfz/util/IniParser.hpp>

namespace vox {

using std::int32_t;

// ConfigData struct
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct ConfigData {
	// Debug
	bool continuousShaderReload;
	bool printFrametimes;

	// Graphics
	int32_t displayIndex;
	int32_t fullscreenMode; // 0 = off, 1 = windowed, 2 = exclusive
	int32_t refreshRate, resolutionX, resolutionY; // DisplayMode
	int32_t windowWidth, windowHeight;
	int32_t vsync; // 0 = off, 1 = on, 2 = swap control tear
	float internalResScaling;
	float ssaoResScaling;
	float spotlightResScaling;
	float lightShaftsResScaling;
	int32_t scalingAlgorithm;

	// Voxel
	int32_t verticalRange, horizontalRange;
};

bool operator== (const ConfigData& lhs, const ConfigData& rhs) noexcept;
bool operator!= (const ConfigData& lhs, const ConfigData& rhs) noexcept;

// GlobalConfig class
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class GlobalConfig final : public ConfigData {
public:
	// Singleton instance
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	static GlobalConfig& INSTANCE() noexcept;

	// Loading and saving to file
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void load() noexcept;
	void save() noexcept;

	// ConfigData getters & setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline ConfigData data() const noexcept { return *this; }
	void data(const ConfigData& configData) noexcept;

private:
	// Private constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GlobalConfig(const GlobalConfig&) = default;
	GlobalConfig& operator= (const GlobalConfig&) = default;
	~GlobalConfig() noexcept = default;

	GlobalConfig() noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	sfz::IniParser mIniParser;
};

} // namespace vox

#endif