#include "screens/BaseGameScreen.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

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

void drawWorld(const World& world, const Assets& assets, GLuint shader)
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

void drawLight(const vox::Assets& assets, GLuint shader, const vec3f& lightPos)
{
	static vox::CubeObject cubeObj;
	sfz::mat4f transform = sfz::identityMatrix4<float>();

	// Render sun
	sfz::translation(transform, lightPos);
	gl::setUniform(shader, "modelMatrix", transform);
	glBindTexture(GL_TEXTURE_2D, assets.YELLOW.mHandle);
	cubeObj.render();
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

BaseGameScreen::BaseGameScreen(sdl::Window& window, const std::string& worldName)
:
	mWorld{worldName},
	mCam{},

	mWindow{window},
	mAssets{},

	mShaderProgram{vox::compileStandardShaderProgram()},
	mShadowMapShaderProgram{vox::compileShadowMapShaderProgram()},
	mPostProcessShaderProgram{vox::compilePostProcessShaderProgram()},
	mShadowMap{4096, vox::ShadowMapRes::BITS_32, true, sfz::vec4f{1.f, 1.f, 1.f, 1.f}},
	
	mBaseFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mPostProcessedFramebuffer{window.drawableWidth(), window.drawableHeight()}
{

	float aspect = static_cast<float>(window.width()) / static_cast<float>(window.height());
	projMatrix = sfz::glPerspectiveProjectionMatrix(mCam.mFov, aspect, 0.5f, 1000.0f);

	lightPosSpherical = vec3f{60.0f, sfz::g_PI_FLOAT*0.15f, sfz::g_PI_FLOAT*0.35f}; // [0] = r, [1] = theta, [2] = phi
	lightTarget = vec3f{16.0f, 0.0f, 16.0f};
	lightColor = vec3f{1.0f, 1.0f, 1.0f};
}

/*BaseGameScreen::BaseGameScreen(const BaseGameScreen& baseGameScreen)
{

}*/

BaseGameScreen::~BaseGameScreen()
{

}

// Overriden methods from IScreen
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void BaseGameScreen::update(const std::vector<SDL_Event>& events,
                            const sdl::GameController& ctrl, float delta)
{
	for (auto& event : events) {
		switch (event.type) {
		case SDL_QUIT: quitApplication(); return;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				float w = static_cast<float>(event.window.data1);
				float h = static_cast<float>(event.window.data2);
				projMatrix = sfz::glPerspectiveProjectionMatrix(mCam.mFov, w/h, 0.5f, 1000.0f);
				reloadFramebuffers(event.window.data1, event.window.data2);
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: quitApplication(); return;
			case 'w':
			case 'W':
				mCam.mPos += (mCam.mDir * 25.0f * delta);
				break;
			case 's':
			case 'S':
				mCam.mPos -= (mCam.mDir * 25.0f * delta);
				break;
			case 'a':
			case 'A':
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				mCam.mPos += (-right * 25.0f * delta);}
				break;
			case 'd':
			case 'D':
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				mCam.mPos += (right * 25.0f * delta);}
				break;
			case 'q':
			case 'Q':
				mCam.mPos += (sfz::vec3f{0.0f,-1.0f,0.0} * 25.0f * delta);
				break;
			case 'e':
			case 'E':
				mCam.mPos += (sfz::vec3f{0.0f,1.0f,0.0} * 25.0f * delta);
				break;
			case SDLK_UP:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::g_PI_FLOAT*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_DOWN:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::g_PI_FLOAT*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_LEFT:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, -1.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::g_PI_FLOAT*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_RIGHT:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 1.0f*sfz::g_PI_FLOAT*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::g_PI_FLOAT*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			}
			break;
		}
	}

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
		sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
		sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, ctrl.mRightStick[0]*turningSpeed*delta);
		sfz::mat3f yTurn = sfz::rotationMatrix3(right, ctrl.mRightStick[1]*turningSpeed*delta);
		mCam.mDir = (yTurn * xTurn * mCam.mDir);
		mCam.mUp = (yTurn * xTurn * mCam.mUp);
	}
	if (ctrl.mLeftStick.norm() > ctrl.mLeftStickDeadzone) {
		sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
		mCam.mPos += ((mCam.mDir * ctrl.mLeftStick[1] + right * ctrl.mLeftStick[0]) * currentSpeed * delta);
	}

	// Shoulder buttons
	if (ctrl.mButtonLeftShoulder == sdl::Button::DOWN || ctrl.mButtonLeftShoulder == sdl::Button::HELD) {
		mCam.mPos -= (sfz::vec3f{0,1,0} * currentSpeed * delta);
	}
	else if (ctrl.mButtonRightShoulder == sdl::Button::DOWN || ctrl.mButtonRightShoulder == sdl::Button::HELD) {
		mCam.mPos += (sfz::vec3f{0,1,0} * currentSpeed * delta);
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
		quitApplication();
	}


	if (currentLightAxis != -1) {
		lightPosSpherical[currentLightAxis] += delta * lightCurrentSpeed;
		lightPosSpherical[currentLightAxis] = std::fmod(lightPosSpherical[currentLightAxis],
		                                                (sfz::g_PI_FLOAT*2.0f));
	}

	mCam.update();
	mWorld.update(mCam.mPos);
}

void BaseGameScreen::render(float delta)
{
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // Accept fragments closer to camera than former ones

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Draw shadow map
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mShadowMapShaderProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap.mFBO);
	glViewport(0, 0, mShadowMap.mResolution, mShadowMap.mResolution);

	// Light position and matrices
	const sfz::vec3f lightPos = sphericalToCartesian(lightPosSpherical);
	const sfz::mat4f lightViewMatrix = sfz::lookAt(lightPos, lightTarget, sfz::vec3f{0.0f, 1.0f, 0.0f});
	const sfz::mat4f lightProjMatrix = sfz::glPerspectiveProjectionMatrix(65.0f, 1.0f, 3.0f, 120.0f);
	
	gl::setUniform(mShadowMapShaderProgram, "viewMatrix", lightViewMatrix);
	gl::setUniform(mShadowMapShaderProgram, "projectionMatrix", lightProjMatrix);

	// Clear shadow map
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fix surface acne
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(5.0f, 25.0f);

	// Draw shadow casters
	drawWorld(mWorld, mAssets, mShadowMapShaderProgram);

	// Cleanup
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw base framebuffer (before post-processing)
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mShaderProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, mBaseFramebuffer.mFrameBufferObject);
	glViewport(0, 0, mBaseFramebuffer.mWidth, mBaseFramebuffer.mHeight);

	// Clearing screen
	glClearColor(0.98f, 0.98f, 0.94f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set view and projection matrix uniforms
	gl::setUniform(mShaderProgram, "viewMatrix", mCam.mViewMatrix);
	gl::setUniform(mShaderProgram, "projectionMatrix", projMatrix);

	// Calculate and set lightMatrix
	sfz::mat4f lightMatrix = sfz::translationMatrix(0.5f, 0.5f, 0.5f)
	                       * sfz::scalingMatrix4(0.5f)
	                       * lightProjMatrix
	                       * lightViewMatrix; // * inverse(viewMatrix), done in vertex shader.
	
	gl::setUniform(mShaderProgram, "lightMatrix", lightMatrix);

	// Set light position uniform
	gl::setUniform(mShaderProgram, "msLightPos", lightPos);
	gl::setUniform(mShaderProgram, "lightColor", lightColor);
	
	// Set shadow map uniforms and textures
	gl::setUniform(mShaderProgram, "shadowMap", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mShadowMap.mDepthTexture);

	// Only one texture is used when rendering SnakeTiles
	gl::setUniform(mShaderProgram, "tex", 0);
	glActiveTexture(GL_TEXTURE0);

	// Drawing objects
	drawWorld(mWorld, mAssets, mShaderProgram);
	drawLight(mAssets, mShaderProgram, lightPos);

	// Applying post-process effects
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mPostProcessShaderProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, mPostProcessedFramebuffer.mFrameBufferObject);
	glViewport(0, 0, mPostProcessedFramebuffer.mWidth, mPostProcessedFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, mBaseFramebuffer.mColorTexture);
	gl::setUniform(mPostProcessShaderProgram, "colorTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, mBaseFramebuffer.mNormalTexture);
	gl::setUniform(mPostProcessShaderProgram, "normalTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, mBaseFramebuffer.mDepthTexture);
	gl::setUniform(mPostProcessShaderProgram, "depthTexture", 2);

	mFullscreenQuad.render();

	glUseProgram(0);

	// Blitting post-processed framebuffer to screen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWindow.drawableWidth(), mWindow.drawableHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mPostProcessedFramebuffer.mFrameBufferObject);
	glBlitFramebuffer(0, 0, mPostProcessedFramebuffer.mWidth, mPostProcessedFramebuffer.mHeight,
	                  0, 0, mWindow.drawableWidth(), mWindow.drawableHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

std::unique_ptr<IScreen> BaseGameScreen::changeScreen()
{
	return std::move(std::unique_ptr<IScreen>{mNewScreenPtr});
}

bool BaseGameScreen::quit()
{
	return mQuit;
}

// Protected methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void BaseGameScreen::quitApplication()
{
	mQuit = true;
}

void BaseGameScreen::changeScreen(IScreen* newScreen)
{
	mNewScreenPtr = newScreen;
}

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void BaseGameScreen::reloadFramebuffers(int width, int height)
{
	mBaseFramebuffer = BigFramebuffer{width, height};
	mPostProcessedFramebuffer = Framebuffer{width, height};
}

} // namespace vox