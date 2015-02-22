#include <chrono>
#include <iostream>

#include "sfz/GL.hpp"
#undef main

#include "VoxModel.hpp"
#include "Rendering.hpp"

// Variables
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint shaderProgram;
vox::World world{"test"};
vox::Camera cam;
sfz::mat4f projMatrix;
SDL_GameController* controllers[4];
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
	for (SDL_GameController*& c : controllers) {
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

		controllers[i] = SDL_GameControllerOpen(i);
		if (controllers[i] == NULL) {
			std::cerr << "Couldn't open Game Controller at id " << i << std::endl;
			continue;
		}

		std::cout << "Added Game Controller (id " << i << "): "
		          << SDL_GameControllerName(controllers[i]) << std::endl;
	}

	currentController = 0;
	for (int i = 0; i < 4; i++) {
		if (controllers[i] == NULL) break;
		if (strcmp("X360 Controller", SDL_GameControllerName(controllers[i])) == 0) {
			currentController = i;
			break;
		}
	}

	std::cout << "Current active controller: " << currentController << ", type: "
	          << SDL_GameControllerName(controllers[currentController]) << std::endl;
}

// Game loop functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Called once for each event every frame.
bool handleInput(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_QUIT: return true;
	case SDL_WINDOWEVENT:
		switch (event.window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			float w = static_cast<float>(event.window.data1);
			float h = static_cast<float>(event.window.data2);
			projMatrix = sfz::glPerspectiveProjectionMatrix(cam.mFov, w/h, 0.1f, 1000.0f);
			break;
		}
		break;
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE: return true;
		case 'w':
		case 'W':
			cam.mPos += cam.mDir*0.1f;
			break;
		case 's':
		case 'S':
			cam.mPos -= cam.mDir*0.1f;
			break;
		}
	case SDL_CONTROLLERDEVICEADDED:
	case SDL_CONTROLLERDEVICEREMOVED:
	case SDL_CONTROLLERDEVICEREMAPPED:
		addControllers();
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		if (event.cbutton.which != currentController) break;
		std::cout << "Button down (" << event.cbutton.which << "): " << (int)event.cbutton.button << ", name: "
		          << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button)
		          << std::endl;
		break;
	case SDL_CONTROLLERBUTTONUP:
		if (event.cbutton.which != currentController) break;
		std::cout << "Button up (" << event.cbutton.which << "): " << (int)event.cbutton.button << ", name: "
		          << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button)
		          << std::endl;
		break;
	case SDL_CONTROLLERAXISMOTION:
		if (event.caxis.which != currentController) break;
		if (-1200 < event.caxis.value && event.caxis.value < 1200) break;
		std::cout << "Axis " << (int)event.caxis.axis << ", name: "
		          << SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)event.caxis.axis)
		          << ", value: " << event.caxis.value << std::endl;


		//std::cout << "Controller used!\n";
		break;
	}
	return false;
}

// Called once every frame
bool update(float)
{
	cam.update();
	world.update(cam.mPos);
	return false;
}

// Called once every frame
void render(sdl::Window& window, vox::Assets& assets, float)
{
	static vox::CubeObject cubeObj;

	// Clearing screen
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Enable culling
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);

	glViewport(0, 0, window.drawableWidth(), window.drawableHeight());

	glUseProgram(shaderProgram);

	const sfz::mat4f viewProj = projMatrix * cam.mViewMatrix;

	// Only one texture is used when rendering SnakeTiles
	gl::setUniform(shaderProgram, "tex", 0);
	glActiveTexture(GL_TEXTURE0);

	const vox::Chunk* chunkPtr;
	vox::Offset offset;
	sfz::vec3f offsetVec;
	sfz::mat4f transform = sfz::identityMatrix4<float>();
	bool fullChunk;

	for (size_t i = 0; i < world.numChunks(); i++) {
		chunkPtr = world.chunkPtr(i);
		if (chunkPtr == nullptr) continue;
		if (chunkPtr->isEmptyChunk()) continue;

		offset = world.chunkOffset(chunkPtr);
		offsetVec = world.positionFromChunkOffset(offset);
		fullChunk = chunkPtr->isFullChunk();


		for (size_t y = 0; y < vox::CHUNK_SIZE; y++) {
			if (chunkPtr->isEmptyLayer(y)) continue;
			for (size_t z = 0; z < vox::CHUNK_SIZE; z++) {
				if (chunkPtr->isEmptyXRow(y, z)) continue;
				for (size_t x = 0; x < vox::CHUNK_SIZE; x++) {

					// Only renders outside of full chunks.
					if (fullChunk && offset != world.currentChunkOffset()) {
						const size_t max = vox::CHUNK_SIZE-1;
						bool yMid = (y != 0 && y != max);
						bool zMid = (z != 0 && z != max);
						bool xMid = (x != 0 && x != max);

						if (yMid && zMid && xMid) continue;
					}

					vox::Voxel v = chunkPtr->getVoxel(y, z, x);
					if (v.type() == vox::VoxelType::AIR) continue;

					sfz::translation(transform, offsetVec + sfz::vec3f{static_cast<float>(x),
					                                                   static_cast<float>(y),
					                                                   static_cast<float>(z)});
					gl::setUniform(shaderProgram, "modelViewProj", viewProj * transform);

					glBindTexture(GL_TEXTURE_2D, assets.getCubeFaceTexture(v));
					cubeObj.render();
				}
			}
		}
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
	sdl::Window window{"MinVox", 800, 600,
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


	// Variables
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	shaderProgram = vox::compileStandardShaderProgram();

	float aspect = static_cast<float>(window.width()) / static_cast<float>(window.height());
	projMatrix = sfz::glPerspectiveProjectionMatrix(cam.mFov, aspect, 0.1f, 1000.0f);

	vox::Assets assets;

	checkGLErrorsMessage("^^^ Above errors caused by initing variables and loading assets.");

	// Game loop
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	bool running = true;
	float delta = calculateDelta(); // Call calculateDelta() here to initialize counting.
	SDL_Event event;

	while (running) {
		delta = calculateDelta();

		//std::cout << "Delta = " << delta << ", fps = " << (1.0f / delta) << "\n";

		while (SDL_PollEvent(&event) != 0) if (handleInput(event)) running = false;
		if (update(delta)) running = false;
		render(window, assets, delta);

		SDL_GL_SwapWindow(window.mPtr);

		checkGLErrorsMessage("^^^ Above errors likely caused by game loop.");
	}

	return 0;
}