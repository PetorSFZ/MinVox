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

	// Small hack to enable hi-dpi awareness on Windows.
#ifdef _WIN32
	SetProcessDPIAware();
#endif

	vox::GlobalConfig& cfg = vox::getGlobalConfig();
	Session sdlSession{{InitFlags::EVENTS, InitFlags::VIDEO, InitFlags::GAMECONTROLLER}};
	Window window{"MinVox", cfg.mWindowResolutionX, cfg.mWindowResolutionY,
	             {WindowFlags::OPENGL, WindowFlags::RESIZABLE, WindowFlags::ALLOW_HIGHDPI, 
	             cfg.mFullscreen ? WindowFlags::FULLSCREEN_DESKTOP : WindowFlags::OPENGL}};

	// Creates OpenGL context, debug if SFZ_NO_DEBUG is not defined
#if !defined(SFZ_NO_DEBUG)
	gl::Context glContext{window.mPtr, 4, 1, gl::GLContextProfile::CORE, true};
#else
	gl::Context glContext{window.mPtr, 4, 1, gl::GLContextProfile::CORE, false};
#endif

	// Initializes GLEW, must happen after GL context is created.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		std::cerr << "GLEW initialization failure:\n" << glewGetErrorString(glewError) << std::endl;
		std::terminate();
	}

	gl::printSystemGLInfo();

	// Enable OpenGL debug message if in debug mode
#if !defined(SFZ_NO_DEBUG)
	gl::setupDebugMessages(gl::Severity::MEDIUM, gl::Severity::HIGH);
#endif

	// Enable/disable vsync
	if (!cfg.mVSync) SDL_GL_SetSwapInterval(0);

	sfz::runGameLoop(window, std::shared_ptr<sfz::BaseScreen>{new vox::GameScreen{window, "test"}});

	return 0;
}