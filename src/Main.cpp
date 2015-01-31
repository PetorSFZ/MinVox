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
	}
	return false;
}

// Called once every frame
bool update(float delta)
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


	const vox::Chunk* chunkPtr = world.chunkPtr(vox::ChunkOffset{0, 0, 0});
	sfz::mat4f transform;

	for (size_t y = 0; y < vox::CHUNK_SIZE; y++) {
		if (chunkPtr->isEmptyLayer(y)) continue;
		for (size_t z = 0; z < vox::CHUNK_SIZE; z++) {
			if (chunkPtr->isEmptyRow(y, z)) continue;
			for (size_t x = 0; x < vox::CHUNK_SIZE; x++) {

				vox::Voxel v = chunkPtr->getVoxel(y, z, x);
				if (v.type() == vox::VoxelType::AIR) continue;

				transform = viewProj
						  * sfz::translationMatrix((float)x, (float)y, (float)z);
				gl::setUniform(shaderProgram, "modelViewProj", transform);

				glBindTexture(GL_TEXTURE_2D, assets.getCubeFaceTexture(v));
				cubeObj.render();
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

	sdl::Session sdlSession{{sdl::InitFlags::EVERYTHING}, {sdl::ImgInitFlags::PNG}};
	sdl::Window window{"snakium³", 800, 600,
	    {sdl::WindowFlags::OPENGL, sdl::WindowFlags::RESIZABLE, sdl::WindowFlags::ALLOW_HIGHDPI}};

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