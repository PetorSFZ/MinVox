#include <iostream>
#include <memory>

#include <sfz/GL.hpp>
#include <sfz/SDL.hpp>
#undef main

#include "Screens.hpp"
#include "GlobalConfig.hpp"

int main()
{
	using namespace sdl;
	using std::string;
	
	// Small hack to enable hi-dpi awareness on Windows.
#ifdef _WIN32
	SetProcessDPIAware();
#endif

	vox::GlobalConfig& cfg = vox::GlobalConfig::INSTANCE();
	cfg.save();

	Session sdlSession{{InitFlags::EVENTS, InitFlags::VIDEO, InitFlags::GAMECONTROLLER}};

	// Make sure selected display index is valid
	const int numDisplays = SDL_GetNumVideoDisplays();
	if (numDisplays < 0) std::cerr << "SDL_GetNumVideoDisplays() failed: " << SDL_GetError() << std::endl;
	if (cfg.displayIndex >= numDisplays) {
		std::cerr << "Display index " << cfg.displayIndex << " is invalid, number of displays is "
		          << numDisplays << ". Resetting to 0." << std::endl;
		cfg.displayIndex = 0;
		cfg.save();
	}

	Window window{"MinVox", cfg.windowWidth, cfg.windowHeight,
	     {WindowFlags::OPENGL, WindowFlags::RESIZABLE, WindowFlags::ALLOW_HIGHDPI}};
	
	// Creates OpenGL context, debug if SFZ_NO_DEBUG is not defined
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	const int MAJOR_VERSION = 4;
	const int MINOR_VERSION = 1;
#if !defined(SFZ_NO_DEBUG)
#ifdef _WIN32
	gl::Context glContext{window.mPtr, MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::COMPATIBILITY, true};
#else
	gl::Context glContext{window.mPtr, MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::CORE, true};
#endif
#else
#ifdef _WIN32
	gl::Context glContext{window.mPtr, MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::COMPATIBILITY, false};
#else
	gl::Context glContext{window.mPtr, MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::CORE, false};
#endif
#endif
	
	gl::printSystemGLInfo();

	// Sets correct displaymode
	SDL_DisplayMode cfgDataMode;
	cfgDataMode.w = cfg.resolutionX;
	cfgDataMode.h = cfg.resolutionY;
	cfgDataMode.format = 0;
	cfgDataMode.refresh_rate = cfg.refreshRate;
	cfgDataMode.driverdata = 0;
	SDL_DisplayMode closest;
	if (SDL_GetClosestDisplayMode(cfg.displayIndex, &cfgDataMode, &closest) == NULL) {
		std::cerr << "SDL_GetClosestDisplayMode() failed: " << SDL_GetError() << std::endl;
	}
	if (SDL_SetWindowDisplayMode(window.mPtr, &closest) < 0) {
		std::cerr << "SDL_SetWindowDisplayMode() failed: " << SDL_GetError() << std::endl;
	}

	// Initializes GLEW, must happen after GL context is created.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		sfz_error(string{"GLEW init failure: "} + ((const char*)glewGetErrorString(glewError)) + "\n");
	}

	// Enable OpenGL debug message if in debug mode
#if !defined(SFZ_NO_DEBUG)
	gl::setupDebugMessages(gl::Severity::MEDIUM, gl::Severity::MEDIUM);
#endif

	// VSync
	int vsyncInterval = 1;
	if (cfg.vsync == 0) vsyncInterval = 0;
	else if (cfg.vsync == 1) vsyncInterval = 1;
	else if (cfg.vsync == 2) vsyncInterval = -1;
	if (SDL_GL_SetSwapInterval(vsyncInterval) < 0) {
		std::cerr << "SDL_GL_SetSwapInterval() failed: " << SDL_GetError() << std::endl;
	}

	// Fullscreen
	int fullscreenFlags = 0;
	if (cfg.fullscreenMode == 0) fullscreenFlags = 0;
	else if (cfg.fullscreenMode == 1) fullscreenFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if (cfg.fullscreenMode == 2) fullscreenFlags = SDL_WINDOW_FULLSCREEN;
	if (SDL_SetWindowFullscreen(window.mPtr, fullscreenFlags) < 0) {
		std::cerr << "SDL_SetWindowFullscreen() failed: " << SDL_GetError() << std::endl;
	}

	sfz::runGameLoop(window, std::shared_ptr<sfz::BaseScreen>{new vox::GameScreen{window, "test"}});

	return 0;
}