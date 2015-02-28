#include <chrono>
#include <iostream>

#include "sfz/GL.hpp"
#undef main
#include <sfz/Assert.hpp>
#include <sfz/Math.hpp>

#include "VoxModel.hpp"
#include "Rendering.hpp"

// Structs that really shouldn't be here.
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct ShadowMap {

	GLuint mFBO, mDepthTexture;
	int mResolution;
	bool mHasPCF;

	enum class DepthRes : GLint {
		BITS_16 = GL_DEPTH_COMPONENT16,
		BITS_24 = GL_DEPTH_COMPONENT24,
		BITS_32 = GL_DEPTH_COMPONENT32
	};

	ShadowMap() = default;

	ShadowMap(int resolution, DepthRes depthRes, bool pcf, const sfz::vec4f& borderColor)
	:
		mResolution{resolution},
		mHasPCF{pcf}
	{
		// Generate framebuffer
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		// Generates depth texture
		glGenTextures(1, &mDepthTexture);
		glBindTexture(GL_TEXTURE_2D, mDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(depthRes), resolution, resolution, 0,
		             GL_DEPTH_COMPONENT, GL_FLOAT, 0);

		// Set shadowmap texture min & mag filters (enable/disable pcf)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pcf ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pcf ? GL_LINEAR : GL_NEAREST);

		// Set texture wrap mode to CLAMP_TO_BORDER and set border color.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.glPtr());

		// Magic lines that enable hardware shadowmaps somehow (becomes sampler2Dshadow?)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

		// Bind texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);
		glDrawBuffer(GL_NONE); // No color buffer
		glReadBuffer(GL_NONE);

		// Check that framebuffer is okay
		sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

		// Cleanup
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

struct Framebuffer final {
	GLuint mFBO, mColorTexture, mDepthBuffer;
	int mWidth, mHeight;

	Framebuffer() = default;

	Framebuffer(int width, int height)
	{
		// Color texture
		glGenTextures(1, &mColorTexture);
		glBindTexture(GL_TEXTURE_RECTANGLE, mColorTexture);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		
		// Generate framebuffer
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, mColorTexture, 0);

		// Depth Buffer
		glGenRenderbuffers(1, &mDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);

		// Check that framebuffer is okay
		sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

		// Cleanup
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

// Variables
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint shaderProgram, shadowMapShaderProgram;
Framebuffer internalFramebuffer;
ShadowMap shadowMap;

vox::World world{"test"};
vox::Camera cam;
sfz::mat4f projMatrix;
sfz::vec3f lightPosSpherical{60.0f, sfz::g_PI_FLOAT*0.15f, sfz::g_PI_FLOAT*0.35f}; // [0] = r, [1] = theta, [2] = phi
sfz::vec3f lightTarget{16.0f, 0.0f, 16.0f};
sfz::vec3f lightColor{1.0f, 1.0f, 1.0f};
int currentLightAxis = 1;
float lightCurrentSpeed = 1.0f;
float lightNormalSpeed = 0.5f;
float lightMaxSpeed = sfz::g_PI_FLOAT;


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

sfz::vec3f sphericalToCartesian(float r, float theta, float phi)
{
	using std::sinf;
	using std::cosf;
	return sfz::vec3f{r*sinf(theta)*sinf(phi), r*cosf(phi), r*cosf(theta)*sinf(phi)};
}

sfz::vec3f sphericalToCartesian(const sfz::vec3f& spherical)
{
	return sphericalToCartesian(spherical[0], spherical[1], spherical[2]);
}

// Game loop functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// Called once every frame.
bool handleInputs(float delta)
{
	// Starts updating controller structs
	for (size_t i = 0; i < 4; i++) {
		if (controllerPtrs[i] == NULL) break;
		sdl::updateStart(controllers[i]);
	}

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
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
				cam.mPos += (cam.mDir * 25.0f * delta);
				break;
			case 's':
			case 'S':
				cam.mPos -= (cam.mDir * 25.0f * delta);
				break;
			case SDLK_UP:
				{sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::g_PI_FLOAT*delta);
				cam.mDir = (yTurn * xTurn * cam.mDir);
				cam.mUp = (yTurn * xTurn * cam.mUp);}
				break;
			case SDLK_DOWN:
				{sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::g_PI_FLOAT*delta);
				cam.mDir = (yTurn * xTurn * cam.mDir);
				cam.mUp = (yTurn * xTurn * cam.mUp);}
				break;
			case SDLK_LEFT:
				{sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, -1.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::g_PI_FLOAT*delta);
				cam.mDir = (yTurn * xTurn * cam.mDir);
				cam.mUp = (yTurn * xTurn * cam.mUp);}
				break;
			case SDLK_RIGHT:
				{sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 1.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::g_PI_FLOAT*delta);
				cam.mDir = (yTurn * xTurn * cam.mDir);
				cam.mUp = (yTurn * xTurn * cam.mUp);}
				break;
			}
			break;
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

	if (controllerPtrs[currentController] == NULL) return false;

	sdl::GameController& ctrl = controllers[currentController];
	float currentSpeed = 3.0f;
	float turningSpeed = sfz::g_PI_FLOAT;

	// Triggers
	if (ctrl.mLeftTrigger > ctrl.mLeftTriggerDeadzone) {
		currentSpeed += (ctrl.mLeftTrigger * 12.0f);
	}
	if (ctrl.mRightTrigger > ctrl.mRightTriggerDeadzone) {
		lightCurrentSpeed = ctrl.mRightTrigger * lightMaxSpeed;
	} else {
		lightCurrentSpeed = lightNormalSpeed;
	}

	// Analogue Sticks
	if (ctrl.mRightStick.norm() > ctrl.mRightStickDeadzone) {
		sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
		sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, ctrl.mRightStick[0]*turningSpeed*delta);
		sfz::mat3f yTurn = sfz::rotationMatrix3(right, ctrl.mRightStick[1]*turningSpeed*delta);
		cam.mDir = (yTurn * xTurn * cam.mDir);
		cam.mUp = (yTurn * xTurn * cam.mUp);
	}
	if (ctrl.mLeftStick.norm() > ctrl.mLeftStickDeadzone) {
		sfz::vec3f right = sfz::cross(cam.mDir, cam.mUp).normalize();
		cam.mPos += ((cam.mDir * ctrl.mLeftStick[1] + right * ctrl.mLeftStick[0]) * currentSpeed * delta);
	}

	// Shoulder buttons
	if (ctrl.mButtonLeftShoulder == sdl::Button::DOWN || ctrl.mButtonLeftShoulder == sdl::Button::HELD) {
		cam.mPos -= (sfz::vec3f{0,1,0} * currentSpeed * delta);
	}
	else if (ctrl.mButtonRightShoulder == sdl::Button::DOWN || ctrl.mButtonRightShoulder == sdl::Button::HELD) {
		cam.mPos += (sfz::vec3f{0,1,0} * currentSpeed * delta);
	}

	// Face buttons
	if (ctrl.mButtonY == sdl::Button::UP) {
		if (currentLightAxis != -1) currentLightAxis = currentLightAxis == 1 ? 2 : 1;
	}
	if (ctrl.mButtonX == sdl::Button::UP) {
		currentLightAxis = 1;
	}
	if (ctrl.mButtonB == sdl::Button::UP) {
		currentLightAxis = 2;
	}
	if (ctrl.mButtonA == sdl::Button::UP) {
		currentLightAxis = -1;
	}

	// Menu buttons
	if (ctrl.mButtonBack == sdl::Button::UP) {
		return true;
	}

	return false;
}

// Called once every frame
bool update(float delta)
{
	cam.update();
	world.update(cam.mPos);

	if (currentLightAxis != -1) {
		lightPosSpherical[currentLightAxis] += delta * lightCurrentSpeed;
		lightPosSpherical[currentLightAxis] = std::fmod(lightPosSpherical[currentLightAxis],
		                                                (sfz::g_PI_FLOAT*2.0f));
	}

	return false;
}

void drawWorld(const vox::Assets& assets, GLuint shader);
void drawLight(const vox::Assets& assets, GLuint shader);

// Called once every frame
void render(sdl::Window& window, vox::Assets& assets, float)
{
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // Accept fragments closer to camera than former ones

	// Enable culling
	glEnable(GL_CULL_FACE);

	// DRAW SHADOW MAP
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(shadowMapShaderProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.mFBO);
	glViewport(0, 0, shadowMap.mResolution, shadowMap.mResolution);

	// Light position and matrices
	const sfz::vec3f lightPos = sphericalToCartesian(lightPosSpherical);
	const sfz::mat4f lightViewMatrix = sfz::lookAt(lightPos, lightTarget, sfz::vec3f{0.0f, 1.0f, 0.0f});
	const sfz::mat4f lightProjMatrix = sfz::glPerspectiveProjectionMatrix(70.0f, 1.0f, 2.0f, 250.0f);
	
	gl::setUniform(shadowMapShaderProgram, "viewMatrix", lightViewMatrix);
	gl::setUniform(shadowMapShaderProgram, "projectionMatrix", lightProjMatrix);

	// Clear shadow map
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fix surface acne
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 20.0f);

	// Draw shadow casters
	drawWorld(assets, shadowMapShaderProgram);

	// Cleanup
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// END DRAW SHADOW MAP
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(shaderProgram);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, window.drawableWidth(), window.drawableHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, internalFramebuffer.mFBO);
	glViewport(0, 0, internalFramebuffer.mWidth, internalFramebuffer.mHeight);

	// Clearing screen
	glClearColor(0.98f, 0.98f, 0.94f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set view and projection matrix uniforms
	gl::setUniform(shaderProgram, "viewMatrix", cam.mViewMatrix);
	gl::setUniform(shaderProgram, "projectionMatrix", projMatrix);

	// Calculate and set lightMatrix
	sfz::mat4f lightMatrix = sfz::translationMatrix(0.5f, 0.5f, 0.5f)
	                       * sfz::scalingMatrix4(0.5f)
	                       * lightProjMatrix
	                       * lightViewMatrix; // * inverse(viewMatrix), done in vertex shader.
	
	gl::setUniform(shaderProgram, "lightMatrix", lightMatrix);

	// Set light position uniform
	gl::setUniform(shaderProgram, "msLightPos", lightPos);
	gl::setUniform(shaderProgram, "lightColor", lightColor);
	
	// Set shadow map uniforms and textures
	gl::setUniform(shaderProgram, "shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowMap.mDepthTexture);

	// Only one texture is used when rendering SnakeTiles
	gl::setUniform(shaderProgram, "tex", 0);
	glActiveTexture(GL_TEXTURE0);

	drawWorld(assets, shaderProgram);
	drawLight(assets, shaderProgram);

	// Drawing internal framebuffer to screen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window.drawableWidth(), window.drawableHeight());
	//glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, internalFramebuffer.mFBO);
	/*glBlitFramebuffer(0, 0, internalFramebuffer.mWidth, internalFramebuffer.mHeight,
	                  0, 0, window.drawableWidth(), window.drawableHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);*/
	glBlitFramebuffer(0, 0, 800, 800,
	                  0, 0, 800, 800,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void drawWorld(const vox::Assets& assets, GLuint shader)
{
	static vox::CubeObject cubeObj;
	const vox::Chunk* chunkPtr;
	vox::Offset offset;
	sfz::vec3f offsetVec;
	sfz::mat4f transform = sfz::identityMatrix4<float>();
	bool fullChunk;

	for (size_t i = 0; i < world.numChunks(); i++) {
		chunkPtr = world.chunkPtr(i);
		if (chunkPtr == nullptr) continue;
		//if (chunkPtr->isEmptyChunk()) continue;

		offset = world.chunkOffset(chunkPtr);
		offsetVec = world.positionFromChunkOffset(offset);
		fullChunk = chunkPtr->isFullChunk();


		for (size_t y = 0; y < vox::CHUNK_SIZE; y++) {
			//if (chunkPtr->isEmptyLayer(y)) continue;
			for (size_t z = 0; z < vox::CHUNK_SIZE; z++) {
				//if (chunkPtr->isEmptyXRow(y, z)) continue;
				for (size_t x = 0; x < vox::CHUNK_SIZE; x++) {

					// Only renders outside of full chunks.
					/*if (fullChunk && offset != world.currentChunkOffset()) {
						const size_t max = vox::CHUNK_SIZE-1;
						bool yMid = (y != 0 && y != max);
						bool zMid = (z != 0 && z != max);
						bool xMid = (x != 0 && x != max);

						if (yMid && zMid && xMid) continue;
					}*/

					vox::Voxel v = chunkPtr->getVoxel(y, z, x);
					if (v.type() == vox::VoxelType::AIR) continue;

					sfz::translation(transform, offsetVec + sfz::vec3f{static_cast<float>(x),
					                                                   static_cast<float>(y),
					                                                   static_cast<float>(z)});
					gl::setUniform(shader, "modelMatrix", transform);

					glBindTexture(GL_TEXTURE_2D, assets.getCubeFaceTexture(v));
					cubeObj.render();
				}
			}
		}
	}
}

void drawLight(const vox::Assets& assets, GLuint shader)
{
	static vox::CubeObject cubeObj;
	sfz::mat4f transform = sfz::identityMatrix4<float>();

	// Render sun
	sfz::translation(transform, sphericalToCartesian(lightPosSpherical));
	gl::setUniform(shader, "modelMatrix", transform);
	glBindTexture(GL_TEXTURE_2D, assets.YELLOW.mHandle);
	cubeObj.render();
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


	// Variables
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	shaderProgram = vox::compileStandardShaderProgram();
	shadowMapShaderProgram = vox::compileShadowMapShaderProgram();

	internalFramebuffer = Framebuffer{1024, 1024};
	shadowMap = ShadowMap{1024, ShadowMap::DepthRes::BITS_32, true, sfz::vec4f{1.f, 1.f, 1.f, 1.f}};

	float aspect = static_cast<float>(window.width()) / static_cast<float>(window.height());
	projMatrix = sfz::glPerspectiveProjectionMatrix(cam.mFov, aspect, 0.1f, 1000.0f);

	vox::Assets assets;

	checkGLErrorsMessage("^^^ Above errors caused by initing variables and loading assets.");

	// Game loop
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	bool running = true;
	float delta = calculateDelta(); // Call calculateDelta() here to initialize counting.

	while (running) {
		delta = calculateDelta();

		//std::cout << "Delta = " << delta << ", fps = " << (1.0f / delta) << "\n";

		if (handleInputs(delta)) running = false;
		if (update(delta)) running = false;
		render(window, assets, delta);

		SDL_GL_SwapWindow(window.mPtr);

		checkGLErrorsMessage("^^^ Above errors likely caused by game loop.");
	}

	return 0;
}