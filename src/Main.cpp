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

	vox::GlobalConfig& cfg = vox::getGlobalConfig();
	Session sdlSession{{InitFlags::EVENTS, InitFlags::VIDEO, InitFlags::GAMECONTROLLER}};
	Window window{"MinVox", cfg.mWindowResolutionX, cfg.mWindowResolutionY, {WindowFlags::OPENGL,
	 WindowFlags::RESIZABLE, cfg.mRetinaAware ? WindowFlags::ALLOW_HIGHDPI : WindowFlags::OPENGL,
	 cfg.mFullscreen ? WindowFlags::FULLSCREEN_DESKTOP : WindowFlags::OPENGL}};

	gl::Context glContext{window.mPtr, 3, 3, gl::GLContextProfile::CORE};

	// Initializes GLEW, must happen after GL context is created.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		std::cerr << "GLEW initialization failure:\n" << glewGetErrorString(glewError) << std::endl;
		std::terminate();
	}

	// Enable/disable vsync
	if (!cfg.mVSync) SDL_GL_SetSwapInterval(0);

	sfz::runGameLoop(window, std::shared_ptr<sfz::BaseScreen>{new vox::GameScreen{window, "test"}});

	return 0;
}