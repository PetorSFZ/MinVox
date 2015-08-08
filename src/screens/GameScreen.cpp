#include "screens/GameScreen.hpp"



namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

void checkGLErrorsMessage(const std::string& msg)
{
	if (gl::checkAllGLErrors()) std::cerr << msg << std::endl;
}

vec3 sphericalToCartesian(float r, float theta, float phi) noexcept
{
	using std::sinf;
	using std::cosf;
	return vec3{r*sinf(theta)*sinf(phi), r*cosf(phi), r*cosf(theta)*sinf(phi)};
}

vec3 sphericalToCartesian(const vec3& spherical) noexcept
{
	return sphericalToCartesian(spherical[0], spherical[1], spherical[2]);
}

void drawLight(int modelMatrixLoc, const vec3& lightPos) noexcept
{
	static CubeObject cubeObj;
	static vec3 halfLightSize{2.0f, 2.0f, 2.0f};
	mat4 transform = sfz::scalingMatrix4<float>(4.0f);

	// Render sun
	sfz::translation(transform, lightPos - halfLightSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(Voxel{VOXEL_ORANGE}));
	cubeObj.render();
}

void drawSkyCube(int modelMatrixLoc, const Camera& cam) noexcept
{
	static SkyCubeObject skyCubeObj;
	static const vec3 halfSkyCubeSize{400.0f, 400.0f, 400.0f};
	mat4 transform = sfz::scalingMatrix4(800.0f, 800.0f, 800.0f);

	// Render skycube
	sfz::translation(transform, cam.mPos - halfSkyCubeSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(Voxel{VOXEL_VANILLA}));
	skyCubeObj.render();
}

void drawPlacementCube(int modelMatrixLoc, const vec3& pos, Voxel voxel) noexcept
{
	static CubeObject cubeObj;
	gl::setUniform(modelMatrixLoc, sfz::translationMatrix<float>(pos - vec3{0.025f, 0.025f, 0.025f})*sfz::scalingMatrix4<float>(1.05f));
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(voxel));
	cubeObj.render();
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GameScreen::GameScreen(sdl::Window& window, const std::string& worldName)
:
	mCfg{getGlobalConfig()},

	mWorld{worldName, vec3{-3.0f, 1.2f, 0.2f}, mCfg.mHorizontalRange, mCfg.mVerticalRange},
	mCam{vec3{-3.0f, 2.5f, 0.2f}, vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, 75.0f,
	     (float)window.width()/(float)window.height(), 0.55f, 1000.0f},

	mWindow{window},

	mShadowMapShader{compileShadowMapShaderProgram()},
	mGBufferGenShader{compileGBufferGenShaderProgram()},
	mDirLightingStencilShader{compileDirectionalLightingStencilShaderProgram()},
	mDirLightingShader{compileDirectionalLightingShaderProgram()},
	mGlobalLightingShader{compileGlobalLightingShaderProgram()},
	mOutputSelectShader{compileOutputSelectShaderProgram()},
	mShadowMap{mCfg.mShadowResolution, ShadowMapRes::BITS_16, mCfg.mShadowPCF, vec4{0.f, 0.f, 0.f, 1.f}},
	mGBuffer{window.drawableWidth(), window.drawableHeight()},
	mDirLightFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mGlobalLightingFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mOutputSelectFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mSSAO{window.drawableWidth(), window.drawableHeight(), mCfg.mSSAONumSamples, mCfg.mSSAORadius, mCfg.mSSAOExp},
	mWorldRenderer{mWorld},

	mCurrentVoxel{VOXEL_AIR}
	//mSun{vec3{0.0f, 0.0f, 0.0f}, vec3{1.0f, 0.0f, 0.0f}, 3.0f, 80.0f, vec3{0.2f, 0.25f, 0.8f}}
{
	updateResolutions((int)window.drawableWidth(), (int)window.drawableHeight());

	//mLightPosSpherical = vec3{60.0f, sfz::PI()*0.15f, sfz::PI()*0.35f}; // [0] = r, [1] = theta, [2] = phi
	//mLightTarget = vec3{16.0f, 0.0f, 16.0f};

	// First corridor
	vec3 f1Color{0.0f, 0.0f, 1.0f};
	mLights.emplace_back(vec3{-21.430313f, 5.780775f, 5.168257f}, vec3{0.499439f, -0.200375f, 0.842858f}, 0.5f, 20.0f, f1Color);
	mLights.emplace_back(vec3{-21.720879f, 1.155828f, 15.699636f}, vec3{-0.563084f, 0.218246f, -0.797059f}, 0.5f, 20.0f, f1Color);

	// Staircase
	mLights.emplace_back(vec3{-33.711731f, 13.120087f, 32.218548f}, vec3{0.038979f, -0.521176f, -0.852557f}, 0.5f, 40.0f, vec3{0.8f, 0.2f, 0.8f});

	// Second corridor
	vec3 f2Color{0.0f, 1.0f, 0.0f};
	mLights.emplace_back(vec3{-23.068808f, 8.956177f, 33.155720f}, vec3{-0.092388f, -0.226080f, -0.969712f}, 0.5f, 20.0f, f2Color);
	mLights.emplace_back(vec3{-20.271776f, 2.191609f, 26.143528f}, vec3{-0.271371f, 0.962427f, 0.009065f}, 0.5f, 20.0f, f2Color);

	// Balcony
	mLights.emplace_back(vec3{-17.184530f, 10.616333f, 26.045494f}, vec3{0.932476f, -0.361071f, -0.010368f}, 0.5f, 100.0f, vec3{0.4f, 0.5f, 0.9f});

	// Semi-global
	mLights.emplace_back(vec3{46.868477f, 32.830544f, 18.390802f}, vec3{-0.776988f, -0.629503f, 0.004005f}, 35.0f, 120.0f, vec3{0.2f, 0.25f, 0.8f});

	for (auto& light : mLights) {
		mLightMeshes.emplace_back(light.mCam.mVerticalFov, light.mCam.mNear, light.mRange);
	}
}

/*GameScreen::GameScreen(const GameScreen& gameScreen)
{

}*/

/*GameScreen::~GameScreen()
{

}*/

// Overriden methods from BaseScreen
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ScreenUpdateOp GameScreen::update(const vector<SDL_Event>& events,
								  const unordered_map<int32_t, sdl::GameController>& controllers,
								  float delta)
{
	for (auto& event : events) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: return sfz::SCREEN_QUIT;
			case SDLK_F1:
				mCfg.mPrintFPS = !mCfg.mPrintFPS;
				break;
			case 'r':
				mSSAO.radius(mSSAO.radius() - 0.1f);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 't':
				mSSAO.radius(mSSAO.radius() + 0.1f);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 'f':
				mSSAO.occlusionExp(mSSAO.occlusionExp() - 0.1f);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 'g':
				mSSAO.occlusionExp(mSSAO.occlusionExp() + 0.1f);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 'v':
				mSSAO.numSamples(mSSAO.numSamples() - 8);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 'b':
				mSSAO.numSamples(mSSAO.numSamples() + 8);
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Exp=" << mSSAO.occlusionExp() << std::endl;
				break;
			case 'n':
				mLightShaftExposure -= 0.05f;
				if (mLightShaftExposure < 0.0f) mLightShaftExposure = 0.0f;
				std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
				break;
			case 'm':
				mLightShaftExposure += 0.05f;
				if (mLightShaftExposure > 1.0f) mLightShaftExposure = 1.0f;
				std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
				break;

			case '1':
				mRenderMode = 1;
				break;
			case '2':
				mRenderMode = 2;
				break;
			case '3':
				mRenderMode = 3;
				break;
			case '4':
				mRenderMode = 4;
				break;
			case '5':
				mRenderMode = 5;
				break;
			case '6':
				mRenderMode = 6;
				break;
			case '7':
				mRenderMode = 7;
				break;
			case '8':
				mRenderMode = 8;
				break;

			/*case 'l':
				std::random_device rd;
				std::mt19937_64 gen{rd()};
				std::uniform_real_distribution<float> distr{0.0f, 1.0f};
				mSun.mColor = vec3{distr(gen), distr(gen), distr(gen)};
				std::cout << "New random light color: " << mSun.mColor << std::endl;
				break;*/

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
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				mCam.mPos += (-right * 25.0f * delta);}
				break;
			case 'd':
			case 'D':
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				mCam.mPos += (right * 25.0f * delta);}
				break;
			case 'q':
			case 'Q':
				mCam.mPos += (sfz::vec3{0.0f,-1.0f,0.0} * 25.0f * delta);
				break;
			case 'e':
			case 'E':
				mCam.mPos += (sfz::vec3{0.0f,1.0f,0.0} * 25.0f * delta);
				break;
			case 'p':
			case 'P':
				mOldWorldRenderer = !mOldWorldRenderer;
				if (mOldWorldRenderer) std::cout << "Using old (non-meshed) world renderer.\n";
				else std::cout << "Using (meshed) world renderer.\n";
				break;
			case SDLK_UP:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_DOWN:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_LEFT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, -1.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_RIGHT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 1.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_SPACE:
				vec3 pos = mCam.mPos + mCam.mDir * 1.5f;
				Voxel v = mWorld.getVoxel(pos);
				if (v.mType != VOXEL_AIR) mWorld.setVoxel(pos, Voxel{VOXEL_AIR});
				else mWorld.setVoxel(pos, Voxel{VOXEL_ORANGE});
				break;
			}
			break;
		}
		
	}

	if (controllers.find(0) != controllers.end()) {
		const sdl::GameController& ctrl = controllers.at(0);

		float currentSpeed = 3.0f;
		float turningSpeed = sfz::PI();

		// Triggers
		if (ctrl.leftTrigger > ctrl.triggerDeadzone) {
			
		}
		if (ctrl.rightTrigger > ctrl.triggerDeadzone) {
			currentSpeed += (ctrl.rightTrigger * 12.0f);
		}

		// Analogue Sticks
		if (sfz::length(ctrl.rightStick) > ctrl.stickDeadzone) {
			sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
			sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, ctrl.rightStick[0]*turningSpeed*delta);
			sfz::mat3 yTurn = sfz::rotationMatrix3(right, ctrl.rightStick[1]*turningSpeed*delta);
			mCam.mDir = (yTurn * xTurn * mCam.mDir);
			mCam.mUp = (yTurn * xTurn * mCam.mUp);
		}
		if (sfz::length(ctrl.leftStick) > ctrl.stickDeadzone) {
			sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
			mCam.mPos += ((mCam.mDir * ctrl.leftStick[1] + right * ctrl.leftStick[0]) * currentSpeed * delta);
		}

		// Control Pad
		if (ctrl.padUp == sdl::Button::DOWN) {
			mLightShaftExposure += 0.05f;
			if (mLightShaftExposure > 1.0f) mLightShaftExposure = 1.0f;
			std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
		} else if (ctrl.padDown == sdl::Button::DOWN) {
			mLightShaftExposure -= 0.05f;
			if (mLightShaftExposure < 0.0f) mLightShaftExposure = 0.0f;
			std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
		} else if (ctrl.padLeft == sdl::Button::DOWN) {
			if (mCurrentVoxel.mType >= 1) {
				mCurrentVoxel = Voxel(mCurrentVoxel.mType - uint8_t(1));
			}
		} else if (ctrl.padRight == sdl::Button::DOWN) {
			if (mCurrentVoxel.mType < Assets::INSTANCE().numVoxelTypes() - 1) {
				mCurrentVoxel = Voxel(mCurrentVoxel.mType + uint8_t(1));
			}
		}

		// Shoulder buttons
		if (ctrl.leftShoulder == sdl::Button::DOWN || ctrl.leftShoulder == sdl::Button::HELD) {
			mCam.mPos -= (sfz::vec3{0,1,0} * currentSpeed * delta);
		}
		else if (ctrl.rightShoulder == sdl::Button::DOWN || ctrl.rightShoulder == sdl::Button::HELD) {
			mCam.mPos += (sfz::vec3{0,1,0} * currentSpeed * delta);
		}

		auto vPos = mCam.mPos + mCam.mDir * 4.0f;
		mCurrentVoxelPos = vec3{std::floorf(vPos[0]), std::floorf(vPos[1]), std::floorf(vPos[2])};

		// Face buttons
		if (ctrl.y == sdl::Button::UP) {
			std::random_device rd;
			std::mt19937_64 gen{rd()};
			std::uniform_real_distribution<float> distr{0.0f, 1.0f};
			mLights.emplace_back(mCam.mPos, mCam.mDir, 0.5f, 40.0f, vec3{distr(gen), distr(gen), distr(gen)});
			mLightMeshes.emplace_back(mLights.back().mCam.mVerticalFov, mLights.back().mCam.mNear, mLights.back().mCam.mFar);
			std::cout << "Light: Pos: " << mLights.back().mCam.mPos << ", Dir: " << mLights.back().mCam.mDir << ", Color: " << mLights.back().mColor << std::endl;
		}
		if (ctrl.x == sdl::Button::UP) {
			if (mLights.size() > 0) {
				mLights.pop_back();
				mLightMeshes.pop_back();
			}
		}
		if (ctrl.b == sdl::Button::UP) {
			mWorld.setVoxel(mCurrentVoxelPos, Voxel{VOXEL_AIR});
		}
		if (ctrl.a == sdl::Button::UP) {
			mWorld.setVoxel(mCurrentVoxelPos, mCurrentVoxel);
		}

		// Menu buttons
		if (ctrl.back == sdl::Button::UP) {
			return sfz::SCREEN_QUIT;
		}

		mCam.mUp = vec3{0.0f, 1.0f, 0.0f};
	}

	/*if (currentLightAxis != -1) {
		mLightPosSpherical[currentLightAxis] += delta * lightCurrentSpeed;
		mLightPosSpherical[currentLightAxis] = std::fmod(mLightPosSpherical[currentLightAxis],
		                                                (sfz::PI()*2.0f));
	}*/

	mCam.updateMatrices();
	mCam.updatePlanes();
	mWorld.update(mCam.mPos);

	return sfz::SCREEN_NO_OP;
}

void GameScreen::render(float delta)
{
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // Accept fragments closer to camera than former ones

	// Enable culling
	glEnable(GL_CULL_FACE);

	checkGLErrorsMessage("^^^ Errors caused by: render() setup.");

	// Draw GBuffer
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mGBufferGenShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer.mFBO);
	glViewport(0, 0, mGBuffer.mWidth, mGBuffer.mHeight);

	// Clearing screen
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set view and projection matrix uniforms
	gl::setUniform(mGBufferGenShader, "uViewMatrix", mCam.mViewMatrix);
	gl::setUniform(mGBufferGenShader, "uProjectionMatrix", mCam.mProjMatrix);

	// Prepare for binding the diffuse textures
	gl::setUniform(mGBufferGenShader, "uDiffuseTexture", 0);
	glActiveTexture(GL_TEXTURE0);

	// Drawing objects
	int modelMatrixLocGBufferGen = glGetUniformLocation(mGBufferGenShader, "uModelMatrix");

	gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
	gl::setUniform(mGBufferGenShader, "uEmissive", vec3{0.15f, 0.15f, 0.2f});
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3{0.0f, 0.0f, 0.0f});
	drawSkyCube(modelMatrixLocGBufferGen, mCam);

	gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
	gl::setUniform(mGBufferGenShader, "uEmissive", vec3{0.0f, 0.0f, 0.0f});
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3{1.0, 0.50, 0.25});
	if (!mOldWorldRenderer) mWorldRenderer.drawWorld(mCam, modelMatrixLocGBufferGen);
	else mWorldRenderer.drawWorldOld(mCam, modelMatrixLocGBufferGen);

	/*gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
	gl::setUniform(mGBufferGenShader, "uEmissive", mSun.mColor*0.5f);
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3{0.0f, 0.0f, 0.0f});
	drawLight(modelMatrixLocGBufferGen, mSun.mCam.mPos);*/
	
	if (mCurrentVoxel.mType != VOXEL_AIR && mCurrentVoxel.mType != VOXEL_LIGHT) {
		gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
		gl::setUniform(mGBufferGenShader, "uEmissive", vec3{0.15f, 0.15f, 0.15f});
		gl::setUniform(mGBufferGenShader, "uMaterial", vec3{1.0, 0.50, 0.25});
		drawPlacementCube(modelMatrixLocGBufferGen, mCurrentVoxelPos, mCurrentVoxel);
	}

	checkGLErrorsMessage("^^^ Errors caused by: render() GBuffer.");

	// Directional Lights (+Shadow Maps)
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	// Clear directional lighting texture
	glUseProgram(mDirLightingShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.mFBO);
	glViewport(0, 0, mDirLightFramebuffer.mWidth, mDirLightFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 inverseViewMatrix = inverse(mCam.mViewMatrix);

	size_t lightIndex = 0;
	for (auto& light : mLights) {
		// Check if light is visible
		//if (!mCam.isVisible(light.mCam)) continue;

		// Shadow map
		glUseProgram(mShadowMapShader);
		glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap.mFBO);
		glViewport(0, 0, mShadowMap.mResolution, mShadowMap.mResolution);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		light.update();
		gl::setUniform(mShadowMapShader, "uViewMatrix", light.mCam.mViewMatrix);
		gl::setUniform(mShadowMapShader, "uProjectionMatrix", light.mCam.mProjMatrix);

		// Fix surface acne
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(5.0f, 25.0f);
		//glCullFace(GL_FRONT);

		// Draw shadow casters
		int modelMatrixLocShadowMap = glGetUniformLocation(mShadowMapShader, "uModelMatrix");
		if (!mOldWorldRenderer) mWorldRenderer.drawWorld(light.mCam, modelMatrixLocShadowMap);
		else mWorldRenderer.drawWorldOld(light.mCam, modelMatrixLocShadowMap);

		// Shadow Map: Cleanup
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glCullFace(GL_BACK);

		
		// Render stencil buffer for light region
		glUseProgram(mDirLightingStencilShader);
		glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.mFBO);
		glViewport(0, 0, mDirLightFramebuffer.mWidth, mDirLightFramebuffer.mHeight);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT); // Clears stencil buffer to 0.

		gl::setUniform(mDirLightingStencilShader, "uModelMatrix", DirectionalLightMesh::generateTransform(light.mCam.mPos, light.mCam.mDir, light.mCam.mUp));
		gl::setUniform(mDirLightingStencilShader, "uViewMatrix", mCam.mViewMatrix);
		gl::setUniform(mDirLightingStencilShader, "uProjectionMatrix", mCam.mProjMatrix);

		glDisable(GL_CULL_FACE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		mLightMeshes[lightIndex].render();
		glEnable(GL_CULL_FACE);


		// Render Light
		glUseProgram(mDirLightingShader);

		// Texture uniforms
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.mDiffuseTexture);
		gl::setUniform(mDirLightingShader, "uDiffuseTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.mPositionTexture);
		gl::setUniform(mDirLightingShader, "uPositionTexture", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.mNormalTexture);
		gl::setUniform(mDirLightingShader, "uNormalTexture", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.mMaterialTexture);
		gl::setUniform(mDirLightingShader, "uMaterialTexture", 3);

		// Shadow map uniform
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, mShadowMap.mDepthTexture);
		gl::setUniform(mDirLightingShader, "uShadowMap", 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.mTexture);
		gl::setUniform(mDirLightingShader, "uDirectionalLightingTexture", 5);

		// Set view matrix uniform
		gl::setUniform(mDirLightingShader, "uViewMatrix", mCam.mViewMatrix);

		// Calculate and set lightMatrix
		gl::setUniform(mDirLightingShader, "uLightMatrix", light.lightMatrix(inverseViewMatrix));

		// Set light position uniform
		gl::setUniform(mDirLightingShader, "uLightPos", light.mCam.mPos);
		gl::setUniform(mDirLightingShader, "uLightRange", light.mRange);
		gl::setUniform(mDirLightingShader, "uLightColor", light.mColor);
	
		gl::setUniform(mDirLightingShader, "uLightShaftExposure", mLightShaftExposure);
		gl::setUniform(mDirLightingShader, "uLightShaftRange", mCfg.mLightShaftRange);
		gl::setUniform(mDirLightingShader, "uLightShaftSamples", mCfg.mLightShaftSamples);

		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass stencil test if not 0.
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		mFullscreenQuad.render();
	
		glDisable(GL_STENCIL_TEST);
		glUseProgram(0);
		lightIndex++;
	}

	checkGLErrorsMessage("^^^ Errors caused by: render() directional lights.");

	// Global Lighting + SSAO + Shadow Map
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint aoTex = mSSAO.calculate(mGBuffer.mPositionTexture, mGBuffer.mNormalTexture,
	                               mCam.mProjMatrix, mCfg.mSSAOClean);

	glUseProgram(mGlobalLightingShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mGlobalLightingFramebuffer.mFBO);
	glViewport(0, 0, mGlobalLightingFramebuffer.mWidth, mGlobalLightingFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mDiffuseTexture);
	gl::setUniform(mGlobalLightingShader, "uDiffuseTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mEmissiveTexture);
	gl::setUniform(mGlobalLightingShader, "uEmissiveTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mMaterialTexture);
	gl::setUniform(mGlobalLightingShader, "uMaterialTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, aoTex);
	gl::setUniform(mGlobalLightingShader, "uAOTexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.mTexture);
	gl::setUniform(mGlobalLightingShader, "uDirectionalLightsTexture", 4);

	gl::setUniform(mGlobalLightingShader, "uAmbientLight", vec3{0.2f, 0.2f, 0.2f});

	mFullscreenQuad.render();
	
	glUseProgram(0);

	checkGLErrorsMessage("^^^ Errors caused by: render() Global Lighting + SSAO + Shadow Map");

	// Rendering some text
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	using gl::HorizontalAlign;
	using gl::VerticalAlign;

	float aspect = (float)mGlobalLightingFramebuffer.mWidth / (float)mGlobalLightingFramebuffer.mHeight;
	vec2 fontWindowDimensions{100.0f * aspect, 100.0f};
	vec2 lightingViewport{(float)mGlobalLightingFramebuffer.mWidth, (float)mGlobalLightingFramebuffer.mHeight};

	float fps = 1.0f/delta;
	if (fps > 10000.0f) fps = 10000.0f; // Small hack
	if (1.0f < fps && fps < 500.0f) {
		float fpsTotal = (mFPSMean * (float)mFPSSamples) + fps;
		mFPSSamples++;
		mFPSMean = fpsTotal / (float)mFPSSamples;
	}

	float fontSize = 2.8f;

	FontRenderer& font = Assets::INSTANCE().mFontRenderer;

	if (mCfg.mPrintFPS) {
		using std::string;
		using std::to_string;
		string deltaString = "Delta: " + to_string(delta*1000.0f) + "ms, Mean: " + to_string(1000.0f / mFPSMean) + "ms";;
		string fpsString = "FPS: " + to_string(fps) + ", Mean: " + to_string(mFPSMean);

		font.horizontalAlign(HorizontalAlign::LEFT);
		font.verticalAlign(VerticalAlign::TOP);

		// Drop shadow
		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		font.write(vec2{1.15f, 99.85f}, fontSize, deltaString);
		font.write(vec2{1.15f, 99.85f - fontSize}, fontSize, fpsString);
		font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport,
						  vec4{0.0f, 0.0f, 0.0f, 1.0f});

		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		font.write(vec2{1.0, 100.0f}, fontSize, deltaString);
		font.write(vec2{1.0, 100.0f - fontSize}, fontSize, fpsString);
		font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport,
						  vec4{1.0f, 0.0f, 1.0f, 1.0f});
	}

	// Draw GUI
	font.horizontalAlign(HorizontalAlign::LEFT);
	font.verticalAlign(VerticalAlign::BOTTOM);

	// Drop shadow
	float xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.2f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.2f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel));
	font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport, vec4{0.0f, 0.0f, 0.0f, 1.0f});

	xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.5f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.5f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel));
	font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport, vec4{1.0f, 1.0f, 1.0f, 1.0f});

	checkGLErrorsMessage("^^^ Errors caused by: render() text rendering.");

	// Output select
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mOutputSelectShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mOutputSelectFramebuffer.mFBO);
	glViewport(0, 0, mOutputSelectFramebuffer.mWidth, mOutputSelectFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGlobalLightingFramebuffer.mTexture);
	gl::setUniform(mOutputSelectShader, "uFinishedTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mDiffuseTexture);
	gl::setUniform(mOutputSelectShader, "uDiffuseTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mPositionTexture);
	gl::setUniform(mOutputSelectShader, "uPositionTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mNormalTexture);
	gl::setUniform(mOutputSelectShader, "uNormalTexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mEmissiveTexture);
	gl::setUniform(mOutputSelectShader, "uEmissiveTexture", 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mMaterialTexture);
	gl::setUniform(mOutputSelectShader, "uMaterialTexture", 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, aoTex);
	gl::setUniform(mOutputSelectShader, "uAOTexture", 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.mTexture);
	gl::setUniform(mOutputSelectShader, "uDirectionalLightsTexture", 7);

	gl::setUniform(mOutputSelectShader, "uRenderMode", mRenderMode);

	mFullscreenQuad.render();
	
	glUseProgram(0);

	checkGLErrorsMessage("^^^ Errors caused by: render() output select.");

	// Blitting post-processed framebuffer to screen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWindow.drawableWidth(), mWindow.drawableHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mOutputSelectFramebuffer.mFBO);
	glBlitFramebuffer(0, 0, mOutputSelectFramebuffer.mWidth, mOutputSelectFramebuffer.mHeight,
	                  0, 0, mWindow.drawableWidth(), mWindow.drawableHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
	checkGLErrorsMessage("^^^ Errors caused by: render() blitting.");
}

void GameScreen::onQuit()
{

}

void GameScreen::onResize(vec2 dimensions)
{
	updateResolutions((int)dimensions.x, (int)dimensions.y);
}

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void GameScreen::updateResolutions(int width, int height) noexcept
{
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);
	mCam.mAspectRatio = w/h;
	if (mCfg.mLockedResolution) {
		int lockedW = static_cast<int>((w/h)*mCfg.mLockedResolutionY);
		reloadFramebuffers(lockedW, mCfg.mLockedResolutionY);
		mSSAO.textureSize(lockedW, mCfg.mLockedResolutionY);
	} else {
		reloadFramebuffers(width, height);
		mSSAO.textureSize(width, height);
	}
}

void GameScreen::reloadFramebuffers(int width, int height) noexcept
{
	mGBuffer = GBuffer{width, height};
	mDirLightFramebuffer = DirectionalLightingFramebuffer{width, height};
	mGlobalLightingFramebuffer = PostProcessFramebuffer{width, height};
	mOutputSelectFramebuffer = PostProcessFramebuffer{width, height};
}

} // namespace vox

