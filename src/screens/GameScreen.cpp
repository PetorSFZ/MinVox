#include "screens/GameScreen.hpp"

#include <sfz/util/IO.hpp>

namespace vox {

// Statics
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

static const uint32_t GBUFFER_DIFFUSE = 0;
static const uint32_t GBUFFER_POSITION = 1;
static const uint32_t GBUFFER_NORMAL = 2;
static const uint32_t GBUFFER_EMISSIVE = 3;
static const uint32_t GBUFFER_MATERIAL = 4;

static vec3 sphericalToCartesian(float r, float theta, float phi) noexcept
{
	using std::sinf;
	using std::cosf;
	return vec3{r*sinf(theta)*sinf(phi), r*cosf(phi), r*cosf(theta)*sinf(phi)};
}

static vec3 sphericalToCartesian(const vec3& spherical) noexcept
{
	return sphericalToCartesian(spherical[0], spherical[1], spherical[2]);
}

static void drawLight(int modelMatrixLoc, const vec3& lightPos) noexcept
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

static void drawSkyCube(int modelMatrixLoc, const ViewFrustum& cam) noexcept
{
	static SkyCubeObject skyCubeObj;
	static const vec3 halfSkyCubeSize{400.0f, 400.0f, 400.0f};
	mat4 transform = sfz::scalingMatrix4(800.0f, 800.0f, 800.0f);

	// Render skycube
	sfz::translation(transform, cam.pos() - halfSkyCubeSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(Voxel{VOXEL_VANILLA}));
	skyCubeObj.render();
}

static void drawPlacementCube(int modelMatrixLoc, const vec3& pos, Voxel voxel) noexcept
{
	static CubeObject cubeObj;
	gl::setUniform(modelMatrixLoc, sfz::translationMatrix<float>(pos - vec3{0.025f, 0.025f, 0.025f})*sfz::scalingMatrix4<float>(1.05f));
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(voxel));
	cubeObj.render();
}

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GameScreen::GameScreen(sdl::Window& window, const std::string& worldName)
:
	mCfg{GlobalConfig::INSTANCE()},

	mWorld{worldName, vec3{-3.0f, 1.2f, 0.2f}, mCfg.horizontalRange, mCfg.verticalRange},
	mCam{vec3{-3.0f, 2.5f, 0.2f}, vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, 75.0f,
	     (float)window.width()/(float)window.height(), 0.55f, 1000.0f},

	mWindow{window},
	//mSSAO{window.drawableWidth(), window.drawableHeight(), mCfg.mSSAONumSamples, mCfg.mSSAORadius, mCfg.mSSAOExp},
	mSSAO{window.drawableWidth(), window.drawableHeight(), 16, 2.0f, 1.1f},
	mWorldRenderer{mWorld},

	mCurrentVoxel{VOXEL_AIR}
	//mSun{vec3{0.0f, 0.0f, 0.0f}, vec3{1.0f, 0.0f, 0.0f}, 3.0f, 80.0f, vec3{0.2f, 0.25f, 0.8f}}
{
	updateResolutions((int)window.drawableWidth(), (int)window.drawableHeight());
	mShadowMap = gl::createShadowMap(sfz::vec2i(1024), gl::FBDepthFormat::F16, true, vec4{0.f, 0.f, 0.f, 1.f});

	mShadowMapShader = Program::fromFile((sfz::basePath() + "assets/shaders/shadow_map.vert").c_str(),
		                                 (sfz::basePath() + "assets/shaders/shadow_map.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindFragDataLocation(shaderProgram, 0, "outFragColor");
	});

	mGBufferGenShader = Program::fromFile((sfz::basePath() + "assets/shaders/gbuffer_gen.vert").c_str(),
	                                      (sfz::basePath() + "assets/shaders/gbuffer_gen.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "positionIn");
		glBindAttribLocation(shaderProgram, 1, "texCoordIn");
		glBindAttribLocation(shaderProgram, 2, "normalIn");
		glBindFragDataLocation(shaderProgram, 0, "fragmentDiffuse");
		glBindFragDataLocation(shaderProgram, 1, "fragmentPosition");
		glBindFragDataLocation(shaderProgram, 2, "fragmentNormal");
		glBindFragDataLocation(shaderProgram, 3, "fragmentEmissive");
		glBindFragDataLocation(shaderProgram, 4, "fragmentMaterial");
	});

	mDirLightingStencilShader = Program::fromFile((sfz::basePath() + "assets/shaders/stencil_shader.vert").c_str(),
	                                              (sfz::basePath() + "assets/shaders/stencil_shader.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "positionIn");
	});

	mDirLightingShader = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/spotlight_shading.frag").c_str());

	mGlobalLightingShader = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/global_shading.frag").c_str());

	mOutputSelectShader = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/output_select.frag").c_str());


	//mLightPosSpherical = vec3{60.0f, sfz::PI()*0.15f, sfz::PI()*0.35f}; // [0] = r, [1] = theta, [2] = phi
	//mLightTarget = vec3{16.0f, 0.0f, 16.0f};

	// First corridor
	/*vec3 f1Color{0.0f, 0.0f, 1.0f};
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
	}*/
}

/*GameScreen::GameScreen(const GameScreen& gameScreen)
{

}*/

/*GameScreen::~GameScreen()
{

}*/

// Overriden methods from BaseScreen
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

UpdateOp GameScreen::update(UpdateState& state)
{
	for (auto& event : state.events) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: return sfz::SCREEN_QUIT;
			case SDLK_F1:
				mCfg.printFrametimes = !mCfg.printFrametimes;
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
			
			case ',':
				mSpotlights.emplace_back(mCam.pos(), mCam.dir(), mCam.verticalFov(), mCam.verticalFov() * 0.95f, 40.0f);
				break;

			case '.':
				if (!mSpotlights.empty()) {
					mSpotlights.pop_back();
				}
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

			case 'w':
			case 'W':
				mCam.setPos(mCam.pos() + (mCam.dir() * 25.0f * state.delta));
				break;
			case 's':
			case 'S':
				mCam.setPos(mCam.pos() - (mCam.dir() * 25.0f * state.delta));
				break;
			case 'a':
			case 'A':
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				mCam.setPos(mCam.pos() - right * 25.0f * state.delta);}
				break;
			case 'd':
			case 'D':
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				mCam.setPos(mCam.pos() + right * 25.0f * state.delta);}
				break;
			case 'q':
			case 'Q':
				mCam.setPos(mCam.pos() + sfz::vec3{0.0f, -1.0f, 0.0} * 25.0f * state.delta);
				break;
			case 'e':
			case 'E':
				mCam.setPos(mCam.pos() + sfz::vec3{0.0f, 1.0f, 0.0} *25.0f * state.delta);
				break;
			case 'p':
			case 'P':
				mOldWorldRenderer = !mOldWorldRenderer;
				if (mOldWorldRenderer) std::cout << "Using old (non-meshed) world renderer.\n";
				else std::cout << "Using (meshed) world renderer.\n";
				break;
			case SDLK_UP:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*state.delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::PI()*state.delta);
				mCam.setDir(yTurn * xTurn * mCam.dir(), yTurn * xTurn * mCam.up());}
				break;
			case SDLK_DOWN:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*state.delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::PI()*state.delta);
				mCam.setDir(yTurn * xTurn * mCam.dir(), yTurn * xTurn * mCam.up());}
				break;
			case SDLK_LEFT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, -1.0f*sfz::PI()*state.delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*state.delta);
				mCam.setDir(yTurn * xTurn * mCam.dir(), yTurn * xTurn * mCam.up());}
				break;
			case SDLK_RIGHT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 1.0f*sfz::PI()*state.delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*state.delta);
				mCam.setDir(yTurn * xTurn * mCam.dir(), yTurn * xTurn * mCam.up()); }
				break;
			case SDLK_SPACE:
				vec3 pos = mCam.pos() + mCam.dir() * 1.5f;
				Voxel v = mWorld.getVoxel(pos);
				if (v.mType != VOXEL_AIR) mWorld.setVoxel(pos, Voxel{VOXEL_AIR});
				else mWorld.setVoxel(pos, Voxel{VOXEL_ORANGE});
				break;
			}
			break;
		}
	}

	if (state.controllers.find(0) != state.controllers.end()) {
		const sdl::GameController& ctrl = state.controllers.at(0);

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
			sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
			sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, ctrl.rightStick[0]*turningSpeed*state.delta);
			sfz::mat3 yTurn = sfz::rotationMatrix3(right, ctrl.rightStick[1]*turningSpeed*state.delta);
			mCam.setDir(yTurn * xTurn * mCam.dir(), yTurn * xTurn * mCam.up());
		}
		if (sfz::length(ctrl.leftStick) > ctrl.stickDeadzone) {
			sfz::vec3 right = sfz::normalize(sfz::cross(mCam.dir(), mCam.up()));
			mCam.setPos(mCam.pos() + ((mCam.dir() * ctrl.leftStick[1] + right * ctrl.leftStick[0]) * currentSpeed * state.delta));
		}

		// Control Pad
		if (ctrl.padUp == sdl::ButtonState::DOWN) {
			
		} else if (ctrl.padDown == sdl::ButtonState::DOWN) {

		} else if (ctrl.padLeft == sdl::ButtonState::DOWN) {
			if (mCurrentVoxel.mType >= 1) {
				mCurrentVoxel = Voxel(mCurrentVoxel.mType - uint8_t(1));
			}
		} else if (ctrl.padRight == sdl::ButtonState::DOWN) {
			if (mCurrentVoxel.mType < Assets::INSTANCE().numVoxelTypes() - 1) {
				mCurrentVoxel = Voxel(mCurrentVoxel.mType + uint8_t(1));
			}
		}

		// Shoulder buttons
		if (ctrl.leftShoulder == sdl::ButtonState::DOWN || ctrl.leftShoulder == sdl::ButtonState::HELD) {
			mCam.setPos(mCam.pos() - sfz::vec3{0, 1, 0} * currentSpeed * state.delta);
		}
		else if (ctrl.rightShoulder == sdl::ButtonState::DOWN || ctrl.rightShoulder == sdl::ButtonState::HELD) {
			mCam.setPos(mCam.pos() + sfz::vec3{0, 1, 0} * currentSpeed * state.delta);
		}

		auto vPos = mCam.pos() + mCam.dir() * 4.0f;
		mCurrentVoxelPos = vec3{std::floorf(vPos[0]), std::floorf(vPos[1]), std::floorf(vPos[2])};

		// Face buttons
		if (ctrl.y == sdl::ButtonState::UP) {
			mSpotlights.emplace_back(mCam.pos(), mCam.dir(), mCam.verticalFov(), mCam.verticalFov() * 0.95f, 40.0f);
		}
		if (ctrl.x == sdl::ButtonState::UP) {
			if (!mSpotlights.empty()) {
				mSpotlights.pop_back();
			}
		}
		if (ctrl.b == sdl::ButtonState::UP) {
			mWorld.setVoxel(mCurrentVoxelPos, Voxel{VOXEL_AIR});
		}
		if (ctrl.a == sdl::ButtonState::UP) {
			mWorld.setVoxel(mCurrentVoxelPos, mCurrentVoxel);
		}

		// Menu buttons
		if (ctrl.back == sdl::ButtonState::UP) {
			return sfz::SCREEN_QUIT;
		}

		mCam.setDir(mCam.dir(), vec3{0.0f, 1.0f, 0.0f});
	}

	mWorld.update(mCam.pos());

	return sfz::SCREEN_NO_OP;
}

void GameScreen::render(UpdateState& state)
{
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // Accept fragments closer to camera than former ones

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Draw GBuffer
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mGBufferGenShader.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer.fbo());
	glViewport(0, 0, mGBuffer.width(), mGBuffer.height());

	// Clearing screen
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set view and projection matrix uniforms
	gl::setUniform(mGBufferGenShader, "uViewMatrix", mCam.viewMatrix());
	gl::setUniform(mGBufferGenShader, "uProjectionMatrix", mCam.projMatrix());

	// Prepare for binding the diffuse textures
	gl::setUniform(mGBufferGenShader, "uDiffuseTexture", 0);
	glActiveTexture(GL_TEXTURE0);

	// Drawing objects
	int modelMatrixLocGBufferGen = glGetUniformLocation(mGBufferGenShader.handle(), "uModelMatrix");

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

	// Directional Lights (+Shadow Maps)
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	// Clear directional lighting texture
	glUseProgram(mDirLightingShader.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.fbo());
	glViewport(0, 0, mDirLightFramebuffer.width(), mDirLightFramebuffer.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 inverseViewMatrix = inverse(mCam.viewMatrix());

	size_t lightIndex = 0;
	for (auto& light : mSpotlights) {
		const auto& lightFrustum = light.viewFrustum();

		// Check if light is visible
		//if (!mCam.isVisible(light.mCam)) continue;

		// Shadow map
		glUseProgram(mShadowMapShader.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap.fbo());
		glViewport(0, 0, mShadowMap.width(), mShadowMap.height());
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gl::setUniform(mShadowMapShader, "uViewMatrix", lightFrustum.viewMatrix());
		gl::setUniform(mShadowMapShader, "uProjectionMatrix", lightFrustum.projMatrix());

		// Fix surface acne
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(5.0f, 25.0f);
		//glCullFace(GL_FRONT);

		// Draw shadow casters
		int modelMatrixLocShadowMap = glGetUniformLocation(mShadowMapShader.handle(), "uModelMatrix");
		if (!mOldWorldRenderer) mWorldRenderer.drawWorld(lightFrustum, modelMatrixLocShadowMap);
		else mWorldRenderer.drawWorldOld(lightFrustum, modelMatrixLocShadowMap);

		// Shadow Map: Cleanup
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glCullFace(GL_BACK);

		
		// Render stencil buffer for light region
		glUseProgram(mDirLightingStencilShader.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.fbo());
		glViewport(0, 0, mDirLightFramebuffer.width(), mDirLightFramebuffer.height());
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT); // Clears stencil buffer to 0.

		gl::setUniform(mDirLightingStencilShader, "uModelMatrix", light.viewFrustumTransform());
		gl::setUniform(mDirLightingStencilShader, "uViewMatrix", mCam.viewMatrix());
		gl::setUniform(mDirLightingStencilShader, "uProjectionMatrix", mCam.projMatrix());

		glDisable(GL_CULL_FACE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		light.renderViewFrustum();
		glEnable(GL_CULL_FACE);


		// Render Light
		glUseProgram(mDirLightingShader.handle());

		// Texture uniforms
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_DIFFUSE));
		gl::setUniform(mDirLightingShader, "uDiffuseTexture", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_POSITION));
		gl::setUniform(mDirLightingShader, "uPositionTexture", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_NORMAL));
		gl::setUniform(mDirLightingShader, "uNormalTexture", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_MATERIAL));
		gl::setUniform(mDirLightingShader, "uMaterialTexture", 3);

		// Shadow map uniform
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, mShadowMap.depthTexture());
		gl::setUniform(mDirLightingShader, "uShadowMap", 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.texture(0));
		gl::setUniform(mDirLightingShader, "uDirectionalLightingTexture", 5);

		// Set view matrix uniform
		gl::setUniform(mDirLightingShader, "uViewMatrix", mCam.viewMatrix());

		// Calculate and set lightMatrix
		gl::setUniform(mDirLightingShader, "uLightMatrix", light.lightMatrix(inverseViewMatrix));

		// Set light position uniform
		gl::setUniform(mDirLightingShader, "uLightPos", lightFrustum.pos());
		gl::setUniform(mDirLightingShader, "uLightRange", lightFrustum.far());
		gl::setUniform(mDirLightingShader, "uLightColor", light.color());
	
		gl::setUniform(mDirLightingShader, "uLightShaftExposure", 0.5f);
		gl::setUniform(mDirLightingShader, "uLightShaftRange", 18.0f);
		gl::setUniform(mDirLightingShader, "uLightShaftSamples", 38);

		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass stencil test if not 0.
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		mFullscreenQuad.render();
	
		glDisable(GL_STENCIL_TEST);
		glUseProgram(0);
		lightIndex++;
	}

	// Global Lighting + SSAO + Shadow Map
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint aoTex = mSSAO.calculate(mGBuffer.texture(GBUFFER_POSITION),
	                               mGBuffer.texture(GBUFFER_NORMAL),
	                               mCam.projMatrix(), false);

	glUseProgram(mGlobalLightingShader.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mGlobalLightingFramebuffer.fbo());
	glViewport(0, 0, mGlobalLightingFramebuffer.width(), mGlobalLightingFramebuffer.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_DIFFUSE));
	gl::setUniform(mGlobalLightingShader, "uDiffuseTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_EMISSIVE));
	gl::setUniform(mGlobalLightingShader, "uEmissiveTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_MATERIAL));
	gl::setUniform(mGlobalLightingShader, "uMaterialTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, aoTex);
	gl::setUniform(mGlobalLightingShader, "uAOTexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.texture(0));
	gl::setUniform(mGlobalLightingShader, "uDirectionalLightsTexture", 4);

	gl::setUniform(mGlobalLightingShader, "uAmbientLight", vec3{0.2f, 0.2f, 0.2f});

	mFullscreenQuad.render();
	
	glUseProgram(0);

	// Rendering some text
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	using gl::HorizontalAlign;
	using gl::VerticalAlign;

	float aspect = mGlobalLightingFramebuffer.widthFloat() / mGlobalLightingFramebuffer.heightFloat();
	vec2 fontWindowDimensions{100.0f * aspect, 100.0f};
	vec2 lightingViewport = mGlobalLightingFramebuffer.dimensionsFloat();

	float fps = 1.0f/state.delta;
	if (fps > 10000.0f) fps = 10000.0f; // Small hack
	if (1.0f < fps && fps < 500.0f) {
		float fpsTotal = (mFPSMean * (float)mFPSSamples) + fps;
		mFPSSamples++;
		mFPSMean = fpsTotal / (float)mFPSSamples;
	}

	float fontSize = 2.8f;

	FontRenderer& font = Assets::INSTANCE().mFontRenderer;

	if (mCfg.printFrametimes) {
		using std::string;
		using std::to_string;
		string deltaString = "Delta: " + to_string(state.delta*1000.0f) + "ms, Mean: " + to_string(1000.0f / mFPSMean) + "ms";;
		string fpsString = "FPS: " + to_string(fps) + ", Mean: " + to_string(mFPSMean);

		font.horizontalAlign(HorizontalAlign::LEFT);
		font.verticalAlign(VerticalAlign::TOP);

		// Drop shadow
		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		font.write(vec2{1.15f, 99.85f}, fontSize, deltaString.c_str());
		font.write(vec2{1.15f, 99.85f - fontSize}, fontSize, fpsString.c_str());
		font.end(mGlobalLightingFramebuffer.fbo(), lightingViewport,
						  vec4{0.0f, 0.0f, 0.0f, 1.0f});

		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		font.write(vec2{1.0, 100.0f}, fontSize, deltaString.c_str());
		font.write(vec2{1.0, 100.0f - fontSize}, fontSize, fpsString.c_str());
		font.end(mGlobalLightingFramebuffer.fbo(), lightingViewport,
						  vec4{1.0f, 0.0f, 1.0f, 1.0f});
	}

	// Draw GUI
	font.horizontalAlign(HorizontalAlign::LEFT);
	font.verticalAlign(VerticalAlign::BOTTOM);

	// Drop shadow
	float xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.2f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.2f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel).c_str());
	font.end(mGlobalLightingFramebuffer.fbo(), lightingViewport, vec4{0.0f, 0.0f, 0.0f, 1.0f});

	xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.5f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.5f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel).c_str());
	font.end(mGlobalLightingFramebuffer.fbo(), lightingViewport, vec4{1.0f, 1.0f, 1.0f, 1.0f});

	// Output select
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mOutputSelectShader.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mOutputSelectFramebuffer.fbo());
	glViewport(0, 0, mOutputSelectFramebuffer.width(), mOutputSelectFramebuffer.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGlobalLightingFramebuffer.texture(0));
	gl::setUniform(mOutputSelectShader, "uFinishedTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_DIFFUSE));
	gl::setUniform(mOutputSelectShader, "uDiffuseTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_POSITION));
	gl::setUniform(mOutputSelectShader, "uPositionTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_NORMAL));
	gl::setUniform(mOutputSelectShader, "uNormalTexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_EMISSIVE));
	gl::setUniform(mOutputSelectShader, "uEmissiveTexture", 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_MATERIAL));
	gl::setUniform(mOutputSelectShader, "uMaterialTexture", 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, aoTex);
	gl::setUniform(mOutputSelectShader, "uAOTexture", 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, mDirLightFramebuffer.texture(0));
	gl::setUniform(mOutputSelectShader, "uDirectionalLightsTexture", 7);

	gl::setUniform(mOutputSelectShader, "uRenderMode", mRenderMode);

	mFullscreenQuad.render();
	
	glUseProgram(0);

	// Blitting post-processed framebuffer to screen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWindow.drawableWidth(), mWindow.drawableHeight());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mOutputSelectFramebuffer.fbo());
	glBlitFramebuffer(0, 0, mOutputSelectFramebuffer.width(), mOutputSelectFramebuffer.height(),
	                  0, 0, mWindow.drawableWidth(), mWindow.drawableHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GameScreen::onQuit()
{

}

void GameScreen::onResize(vec2 dimensions, vec2 drawableDimensions)
{
	updateResolutions((int)dimensions.x, (int)dimensions.y);
}

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void GameScreen::updateResolutions(int width, int height) noexcept
{
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);
	mCam.setAspectRatio(w/h);
	if (false) {
		//int lockedW = static_cast<int>((w/h)*mCfg.mLockedResolutionY);
		//reloadFramebuffers(lockedW, mCfg.mLockedResolutionY);
		//mSSAO.textureSize(lockedW, mCfg.mLockedResolutionY);
	} else {
		reloadFramebuffers(width, height);
		mSSAO.textureSize(width, height);
	}
}

void GameScreen::reloadFramebuffers(int width, int height) noexcept
{
	mGBuffer = gl::FramebufferBuilder{sfz::vec2i{width, height}}
	          .addDepthBuffer(gl::FBDepthFormat::F32)
	          .addTexture(GBUFFER_DIFFUSE, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	          .addTexture(GBUFFER_POSITION, gl::FBTextureFormat::RGB_F32, gl::FBTextureFiltering::LINEAR)
	          .addTexture(GBUFFER_NORMAL, gl::FBTextureFormat::RGB_F32, gl::FBTextureFiltering::LINEAR)
	          .addTexture(GBUFFER_EMISSIVE, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	          .addTexture(GBUFFER_MATERIAL, gl::FBTextureFormat::RGB_F32, gl::FBTextureFiltering::LINEAR)
	          .build();

	mDirLightFramebuffer = gl::FramebufferBuilder{sfz::vec2i{width, height}}
	                      .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	                      .addStencilBuffer()
	                      .build();

	mGlobalLightingFramebuffer = gl::FramebufferBuilder{sfz::vec2i{width, height}}
	                            .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	                            .build();

	mOutputSelectFramebuffer = gl::FramebufferBuilder{sfz::vec2i{width, height}}
	                          .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	                          .build();
}

} // namespace vox

