#include "GlobalConfig.hpp"

#include <iostream>
#include <string>
#include <exception> // std::terminate()

#include <sfz/SDL.hpp>
#include <sfz/util/IO.hpp>

namespace vox {

using std::string;

// Static functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

static const string& basePath() noexcept
{
	static const string BASE_PATH = sfz::gameBaseFolderPath() + "/MinVox";
	return BASE_PATH;
}

static const string& userIniPath() noexcept
{
	static const string USER_INI_PATH = basePath() + "/config.ini";
	return USER_INI_PATH;
}

// ConfigData struct
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

bool operator== (const ConfigData& lhs, const ConfigData& rhs) noexcept
{
	return

	// Debug
	lhs.continuousShaderReload == rhs.continuousShaderReload &&
	lhs.printFrametimes == rhs.printFrametimes &&

	// Graphics
	lhs.displayIndex == rhs.displayIndex &&
	lhs.fullscreenMode == rhs.fullscreenMode &&
	lhs.refreshRate == rhs.refreshRate &&
	lhs.resolutionX == rhs.resolutionX &&
	lhs.resolutionY == rhs.resolutionY &&
	lhs.windowWidth == rhs.windowWidth &&
	lhs.windowHeight == rhs.windowHeight &&
	lhs.vsync == rhs.vsync &&
	lhs.internalResScaling == rhs.internalResScaling &&
	lhs.spotlightResScaling == rhs.spotlightResScaling &&
	lhs.lightShaftsResScaling == rhs.lightShaftsResScaling &&
	lhs.scalingAlgorithm == rhs.scalingAlgorithm &&
	
	// Voxel
	lhs.verticalRange == rhs.verticalRange &&
	lhs.horizontalRange == rhs.horizontalRange;
}

bool operator!= (const ConfigData& lhs, const ConfigData& rhs) noexcept
{
	return !(lhs == rhs);
}

// GlobalConfig: Singleton instance
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GlobalConfig& GlobalConfig::INSTANCE() noexcept
{
	static GlobalConfig cfg = []() {
		GlobalConfig temp;
		temp.load();
		return temp;
	}();
	return cfg;
}

// GlobalConfig: Loading and saving to file
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void GlobalConfig::load() noexcept
{
	if (!sfz::directoryExists(sfz::gameBaseFolderPath().c_str())) {
		sfz::createDirectory(sfz::gameBaseFolderPath().c_str());
	}
	if (!sfz::directoryExists(basePath().c_str())) {
		sfz::createDirectory(basePath().c_str());
	}
	if (!sfz::fileExists(userIniPath().c_str())) {
		sfz::createFile(userIniPath().c_str());
	}

	if (!mIniParser.load()) {
		std::cerr << "Couldn't load config.ini at: " << userIniPath() << std::endl;
		std::terminate();
	}

	sfz::IniParser& ip = mIniParser;
	
	// [Debug]
	static const string dStr = "Debug";
	continuousShaderReload = ip.sanitizeBool(dStr, "bContinuousShaderReload", false);
	printFrametimes =        ip.sanitizeBool(dStr, "bPrintFrametimes", false);

	// [Graphics]
	static const string grStr = "Graphics";
	internalResScaling = ip.sanitizeFloat(grStr, "fInternalResScaling", 2.0f, 0.01f, 10.0f);
	lightShaftsResScaling = ip.sanitizeFloat(grStr, "fLightShaftsResScaling", 0.5f, 0.01f, 10.0f);
	spotlightResScaling = ip.sanitizeFloat(grStr, "fSpotlightResScaling", 1.0f, 0.01f, 10.0f);
	displayIndex =      ip.sanitizeInt(grStr, "iDisplayIndex", 0, 0, 32);
	fullscreenMode =    ip.sanitizeInt(grStr, "iFullscreenMode", 0, 0, 2);
	refreshRate =       ip.sanitizeInt(grStr, "iRefreshRate", 60, 15, 240);
	resolutionX =       ip.sanitizeInt(grStr, "iResolutionX", 1920, 200, 30720);
	resolutionY =       ip.sanitizeInt(grStr, "iResolutionY", 1080, 200, 17280);
	scalingAlgorithm =  ip.sanitizeInt(grStr, "iScalingAlgorithm", 3, 0, 8);
	vsync =             ip.sanitizeInt(grStr, "iVSync", 1, 0, 2);
	windowHeight =      ip.sanitizeInt(grStr, "iWindowHeight", 800, 200, 10000);
	windowWidth =       ip.sanitizeInt(grStr, "iWindowWidth", 800, 200, 10000);

	// [Voxel]
	static const string vStr = "Voxel";
	verticalRange =   ip.sanitizeInt(vStr, "iVerticalRange", 1, 0, 128);
	horizontalRange = ip.sanitizeInt(vStr, "iHorizontalRange", 2, 0, 128);
}

void GlobalConfig::save() noexcept
{
	// [Debug]
	static const string dStr = "Debug";
	mIniParser.setBool(dStr, "bContinuousShaderReload", continuousShaderReload);
	mIniParser.setBool(dStr, "bPrintFrametimes", printFrametimes);

	// [Graphics]
	static const string grStr = "Graphics";
	mIniParser.setFloat(grStr, "fInternalResScaling", internalResScaling);
	mIniParser.setFloat(grStr, "fLightShaftsResScaling", lightShaftsResScaling);
	mIniParser.setFloat(grStr, "fSpotlightResScaling", spotlightResScaling);
	mIniParser.setInt(grStr, "iDisplayIndex", displayIndex);
	mIniParser.setInt(grStr, "iFullscreenMode", fullscreenMode);
	mIniParser.setInt(grStr, "iRefreshRate", refreshRate);
	mIniParser.setInt(grStr, "iResolutionX", resolutionX);
	mIniParser.setInt(grStr, "iResolutionY", resolutionY);
	mIniParser.setInt(grStr, "iScalingAlgorithm", scalingAlgorithm);
	mIniParser.setInt(grStr, "iVSync", vsync);
	mIniParser.setInt(grStr, "iWindowHeight", windowHeight);
	mIniParser.setInt(grStr, "iWindowWidth", windowWidth);

	// [Voxel]
	static const string vStr = "Voxel";
	mIniParser.setInt(vStr, "iVerticalRange", verticalRange);
	mIniParser.setInt(vStr, "iHorizontalRange", horizontalRange);

	if (!mIniParser.save()) {
		std::cerr << "Couldn't save config.ini at: " << userIniPath() << std::endl;
	}
}

// GlobalConfig: ConfigData getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void GlobalConfig::data(const ConfigData& configData) noexcept
{
	// Debug
	this->continuousShaderReload = configData.continuousShaderReload;
	this->printFrametimes = configData.printFrametimes;

	// Graphics
	this->displayIndex = configData.displayIndex;
	this->fullscreenMode = configData.fullscreenMode;
	this->refreshRate = configData.refreshRate;
	this->resolutionX = configData.resolutionX;
	this->resolutionY = configData.resolutionY;
	this->windowWidth = configData.windowWidth;
	this->windowHeight = configData.windowHeight;
	this->vsync = configData.vsync;
	this->internalResScaling = configData.internalResScaling;
	this->spotlightResScaling = configData.spotlightResScaling;
	this->lightShaftsResScaling = configData.lightShaftsResScaling;
	this->scalingAlgorithm = configData.scalingAlgorithm;

	// Voxel
	this->verticalRange = configData.verticalRange;
	this->horizontalRange = configData.horizontalRange;
}

// GlobalConfig: Private constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GlobalConfig::GlobalConfig() noexcept
:
	mIniParser{userIniPath()}
{ }

} // namespace vox