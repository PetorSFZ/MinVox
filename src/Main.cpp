#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "sfz/GL.hpp"
#undef main
#include <sfz/Assert.hpp>
#include <sfz/Math.hpp>

#include "Screens.hpp"

// Variables
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Controllers
SDL_GameController* controllerPtrs[4];
sdl::GameController controllers[4];
int currentController = 0;

// Helper functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

float calculateDelta()
{
	static std::chrono::high_resolution_clock::time_point previousTime, currentTime;

	previousTime = currentTime;
	currentTime = std::chrono::high_resolution_clock::now();

	using FloatSecondDuration = std::chrono::duration<float>;
	return std::chrono::duration_cast<FloatSecondDuration>(currentTime - previousTime).count();
}

void checkGLErrorsMessage(const std::string& msg)
{
	if (gl::checkAllGLErrors()) std::cerr << msg << std::endl;
}

void addControllers()
{
	for (SDL_GameController*& c : controllerPtrs) {
		if (c != NULL) {
			SDL_GameControllerClose(c);
		}
		c = NULL;
	}

	int numJoysticks = SDL_NumJoysticks();
	if (numJoysticks <= 0) {
		std::cerr << "No joystics connected." << std::endl;
		return;
	}
	if (numJoysticks > 4) numJoysticks = 4;
	
	for (int i = 0; i < 4 && i < numJoysticks; i++) {
		if (!SDL_IsGameController(i)) {
			std::cerr << "Joystick id " << i << " is not a Game Controller." << std::endl;
			continue;
		}

		controllerPtrs[i] = SDL_GameControllerOpen(i);
		if (controllerPtrs[i] == NULL) {
			std::cerr << "Couldn't open Game Controller at id " << i << std::endl;
			continue;
		}

		std::cout << "Added Game Controller (id " << i << "): "
		          << SDL_GameControllerName(controllerPtrs[i]) << std::endl;
	}

	currentController = 0;
	for (int i = 0; i < 4; i++) {
		if (controllerPtrs[i] == NULL) break;
		if (strcmp("X360 Controller", SDL_GameControllerName(controllerPtrs[i])) == 0) {
			currentController = i;
			break;
		}
	}

	std::cout << "Current active controller: " << currentController << ", type: "
	          << SDL_GameControllerName(controllerPtrs[currentController]) << std::endl;
}

void updateControllers(const std::vector<SDL_Event>& events)
{
	// Starts updating controller structs
	for (size_t i = 0; i < 4; i++) {
		if (controllerPtrs[i] == NULL) break;
		sdl::updateStart(controllers[i]);
	}

	for (auto& event : events) {
		switch (event.type) {
		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMAPPED:
			addControllers();
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (event.cbutton.which >= 4) break;
			sdl::updateProcessEvent(controllers[event.cbutton.which], event);
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (event.caxis.which >= 4) break;
			sdl::updateProcessEvent(controllers[event.caxis.which], event);
			break;
		}
	}

	// Finish updating controller structs
	for (size_t i = 0; i < 4; i++) {
		if (controllerPtrs[i] == NULL) break;
		sdl::updateFinish(controllers[i]);
	}
}


// Main
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

int main()
{
	// Initialization
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	sdl::Session sdlSession{{sdl::InitFlags::EVENTS, sdl::InitFlags::VIDEO,
	                         sdl::InitFlags::GAMECONTROLLER}, {sdl::ImgInitFlags::PNG}};
	sdl::Window window{"MinVox", 1024, 1024,
	    {sdl::WindowFlags::OPENGL, sdl::WindowFlags::RESIZABLE, sdl::WindowFlags::ALLOW_HIGHDPI}};

	// Enable SDL Events for controllers
	SDL_GameControllerEventState(SDL_ENABLE);
	addControllers();

	gl::Context glContext{window.mPtr, 3, 3, gl::GLContextProfile::CORE};

	// Initializes GLEW, must happen after GL context is created.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		std::cerr << "GLEW initialization failure:\n" << glewGetErrorString(glewError) << std::endl;
		std::terminate();
	}
	checkGLErrorsMessage("^^^ Above errors caused by glewInit().");

	// Game loop
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	addControllers();

	SDL_Event event;
	std::vector<SDL_Event> events{32};

	std::unique_ptr<vox::IScreen> currentScreen;
	currentScreen = std::unique_ptr<vox::IScreen>{new vox::BaseGameScreen{window, "test"}};

	bool running = true;
	float delta = calculateDelta();

	while (running) {
		delta = calculateDelta();
		events.clear();
		while (SDL_PollEvent(&event) != 0) events.push_back(event);
		updateControllers(events);

		currentScreen->update(events, controllers[currentController], delta);
		auto newScreen = currentScreen->changeScreen();
		if (newScreen != nullptr) {
			currentScreen = std::move(newScreen);
			continue;
		}
		if (currentScreen->quit()) running = false;
		currentScreen->render(delta);

		SDL_GL_SwapWindow(window.mPtr);

		checkGLErrorsMessage("^^^ Above errors likely caused by game loop.");
	}

	return 0;
}