#include "screens/GameScreen.hpp"

#include <sfz/util/IO.hpp>

namespace vox {

// Statics
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

static const uint32_t GBUFFER_LINEAR_DEPTH = 0;
static const uint32_t GBUFFER_NORMAL = 1;
static const uint32_t GBUFFER_DIFFUSE = 2;
static const uint32_t GBUFFER_MATERIAL = 3;


/*static vec3 sphericalToCartesian(float r, float theta, float phi) noexcept
{
	using std::sinf;
	using std::cosf;
	return vec3{r*sinf(theta)*sinf(phi), r*cosf(phi), r*cosf(theta)*sinf(phi)};
}

static vec3 sphericalToCartesian(const vec3& spherical) noexcept
{
	return sphericalToCartesian(spherical[0], spherical[1], spherical[2]);
}*/

static void stupidSetSpotlightUniform(const gl::Program& program, const char* name, const Spotlight& spotlight,
                                      const mat4& viewMatrix, const mat4& invViewMatrix) noexcept
{
	using std::snprintf;
	char buffer[128];
	const auto& frustum = spotlight.viewFrustum();
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "vsPos");
	gl::setUniform(program, buffer, transformPoint(viewMatrix, frustum.pos()));
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "vsDir");
	gl::setUniform(program, buffer, normalize(transformDir(viewMatrix, frustum.dir())));
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "color");
	gl::setUniform(program, buffer, spotlight.color());
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "range");
	gl::setUniform(program, buffer, frustum.far());
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "softFovRad");
	gl::setUniform(program, buffer, frustum.verticalFov() * sfz::DEG_TO_RAD());
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "sharpFovRad");
	gl::setUniform(program, buffer, spotlight.sharpFov() * sfz::DEG_TO_RAD());
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "softAngleCos");
	gl::setUniform(program, buffer, std::cos((frustum.verticalFov() / 2.0f) * sfz::DEG_TO_RAD()));
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "sharpAngleCos");
	gl::setUniform(program, buffer, std::cos((spotlight.sharpFov() / 2.0f) * sfz::DEG_TO_RAD()));
	snprintf(buffer, sizeof(buffer), "%s.%s", name, "lightMatrix");
	gl::setUniform(program, buffer, spotlight.lightMatrix(invViewMatrix));
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
	static const vec3 halfSkyCubeSize{250.0f, 250.0f, 250.0f};
	mat4 transform = sfz::scalingMatrix4(500.0f, 500.0f, 500.0f);

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
	mWindow{window},
	mWorld{worldName, vec3{-3.0f, 1.2f, 0.2f}, mCfg.horizontalRange, mCfg.verticalRange},
		
	mSSAO{vec2i{window.drawableWidth(), window.drawableHeight()}, 32, 1.3f},

	mCam{vec3{-3.0f, 2.5f, 0.2f}, vec3{1.0f, 0.0f, 0.0f}, vec3{0.0f, 1.0f, 0.0f}, 75.0f,
	     (float)window.width()/(float)window.height(), 2.0f, 450.0f},

	mWorldRenderer{mWorld},

	mCurrentVoxel{VOXEL_AIR},

	mShortTermPerfStats{20},
	mLongerTermPerfStats{120},
	mLongestTermPerfStats{960}
{
	updateResolutions(window.drawableDimensions());
	updatePrograms();

	mShadowMapHighRes = gl::createShadowMap(vec2i(2048), gl::FBDepthFormat::F32, true, vec4{0.f, 0.f, 0.f, 1.f});
	mShadowMapLowRes = gl::createShadowMap(vec2i(256), gl::FBDepthFormat::F32, true, vec4{0.f, 0.f, 0.f, 1.f});
}

// Overriden methods from BaseScreen
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

UpdateOp GameScreen::update(UpdateState& state)
{
	mShortTermPerfStats.addSample(state.delta);
	mLongerTermPerfStats.addSample(state.delta);
	mLongestTermPerfStats.addSample(state.delta);

	for (auto& event : state.events) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: return sfz::SCREEN_QUIT;
			case SDLK_F1:
				mCfg.printFrametimes = !mCfg.printFrametimes;
				break;
			case 'r':
				mSSAO.radius(std::max(mSSAO.radius() - 0.1f, 0.1f));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			case 't':
				mSSAO.radius(std::min(mSSAO.radius() + 0.1f, 100.0f));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			case 'f':
				mSSAO.occlusionPower(std::max(mSSAO.occlusionPower() - 0.1f, 0.1f));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			case 'g':
				mSSAO.occlusionPower(std::min(mSSAO.occlusionPower() + 0.1f, 5.0f));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			case 'v':
				mSSAO.numSamples(std::max(mSSAO.numSamples() - 8, size_t(8)));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			case 'b':
				mSSAO.numSamples(std::min(mSSAO.numSamples() + 8, size_t(128)));
				std::cout << "SSAO: Samples=" << mSSAO.numSamples() << ", Radius=" << mSSAO.radius() << ", Power=" << mSSAO.occlusionPower() << std::endl;
				break;
			
			case ',':
				mSpotlights.emplace_back(mCam.pos(), mCam.dir(), mCam.verticalFov() * 0.75f, mCam.verticalFov() * 0.6f, 40.0f);
				break;

			case '.':
				if (!mSpotlights.empty()) {
					mSpotlights.pop_back();
				}
				break;

			case '1':
				mOutputSelect = 1;
				break;
			case '2':
				mOutputSelect = 2;
				break;
			case '3':
				mOutputSelect = 3;
				break;
			case '4':
				mOutputSelect = 4;
				break;
			case '5':
				mOutputSelect = 5;
				break;
			case '6':
				mOutputSelect = 6;
				break;
			case '7':
				mOutputSelect = 7;
				break;
			case '8':
				mOutputSelect = 8;
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
			mSpotlights.emplace_back(mCam.pos(), mCam.dir(), mCam.verticalFov() * 0.75f, mCam.verticalFov() * 0.6f, 40.0f);
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

	updateResolutions(mWindow.drawableDimensions());
	if (mCfg.continuousShaderReload) updatePrograms();

	return sfz::SCREEN_NO_OP;
}

void GameScreen::render(UpdateState& state)
{
	// Rendering GBuffer
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Disable alpha blending
	glDisable(GL_BLEND);

	// Enable culling
	glEnable(GL_CULL_FACE);

	// Binding GBufferGen program and GBuffer
	glUseProgram(mGBufferGenProgram.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer.fbo());
	glViewport(0, 0, mGBuffer.width(), mGBuffer.height());

	// Clearing GBuffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// View Matrix and Projection Matrix uniforms
	const mat4 viewMatrix = mCam.viewMatrix();
	const mat4 invViewMatrix = inverse(viewMatrix);
	const mat4 projMatrix = mCam.projMatrix();
	const mat4 invProjMatrix = inverse(mCam.projMatrix());
	gl::setUniform(mGBufferGenProgram, "uViewMatrix", viewMatrix);
	gl::setUniform(mGBufferGenProgram, "uProjMatrix", projMatrix);
	gl::setUniform(mGBufferGenProgram, "uFarPlaneDist", mCam.far());
	
	// Prepare for binding diffuse texture
	gl::setUniform(mGBufferGenProgram, "uDiffuseTexture", 0);
	glActiveTexture(GL_TEXTURE0);

	int modelMatrixLocGBufferGen = glGetUniformLocation(mGBufferGenProgram.handle(), "uModelMatrix");

	gl::setUniform(mGBufferGenProgram, "uMaterial", vec3{0.25f});
	drawSkyCube(modelMatrixLocGBufferGen, mCam);

	gl::setUniform(mGBufferGenProgram, "uMaterial", vec3{1.0, 0.50, 0.25});
	if (!mOldWorldRenderer) mWorldRenderer.drawWorld(mCam, modelMatrixLocGBufferGen);
	else mWorldRenderer.drawWorldOld(mCam, modelMatrixLocGBufferGen);

	if (mCurrentVoxel.mType != VOXEL_AIR && mCurrentVoxel.mType != VOXEL_LIGHT) {
		gl::setUniform(mGBufferGenProgram, "uMaterial", vec3{1.0, 0.50, 0.25});
		drawPlacementCube(modelMatrixLocGBufferGen, mCurrentVoxelPos, mCurrentVoxel);
	}

	// Spotlights (Shadow Map + Shading + Lightshafts)
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	// Binding textures in advance
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_LINEAR_DEPTH));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_NORMAL));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_DIFFUSE));
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_MATERIAL));
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mShadowMapHighRes.depthTexture());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, mShadowMapLowRes.depthTexture());

	glActiveTexture(GL_TEXTURE0);

	// Settings uniforms and clearing framebuffers in advance
	glUseProgram(mSpotlightShadingProgram.handle());
	gl::setUniform(mSpotlightShadingProgram, "uInvProjMatrix", invProjMatrix);
	gl::setUniform(mSpotlightShadingProgram, "uFarPlaneDist", mCam.far());
	gl::setUniform(mSpotlightShadingProgram, "uLinearDepthTexture", 1);
	gl::setUniform(mSpotlightShadingProgram, "uNormalTexture", 2);
	gl::setUniform(mSpotlightShadingProgram, "uDiffuseTexture", 3);
	gl::setUniform(mSpotlightShadingProgram, "uMaterialTexture", 4);
	gl::setUniform(mSpotlightShadingProgram, "uShadowMap", 5);

	glBindFramebuffer(GL_FRAMEBUFFER, mSpotlightShadingFB.fbo());
	glViewport(0, 0, mSpotlightShadingFB.width(), mSpotlightShadingFB.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(mLightShaftsProgram.handle());
	gl::setUniform(mLightShaftsProgram, "uInvProjMatrix", invProjMatrix);
	gl::setUniform(mLightShaftsProgram, "uFarPlaneDist", mCam.far());
	gl::setUniform(mLightShaftsProgram, "uLinearDepthTexture", 1);
	gl::setUniform(mLightShaftsProgram, "uShadowMap", 6);
	
	glBindFramebuffer(GL_FRAMEBUFFER, mLightShaftsFB.fbo());
	glViewport(0, 0, mLightShaftsFB.width(), mLightShaftsFB.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	glDisable(GL_BLEND);

	for (size_t i = 0; i < mSpotlights.size(); ++i) {
		auto& spotlight = mSpotlights[i];
		const auto& lightFrustum = mSpotlights[i].viewFrustum();

		if (!mCam.isVisible(lightFrustum)) continue;


		// Render shadow maps

		glUseProgram(mShadowMapProgram.handle());

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(5.0f, 25.0f);
		//glCullFace(GL_FRONT);

		gl::setUniform(mShadowMapProgram, "uViewProjMatrix", lightFrustum.projMatrix() * lightFrustum.viewMatrix());
		int modelMatrixLocShadowMap = glGetUniformLocation(mShadowMapProgram.handle(), "uModelMatrix");

		glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapHighRes.fbo());
		glViewport(0, 0, mShadowMapHighRes.width(), mShadowMapHighRes.height());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!mOldWorldRenderer) mWorldRenderer.drawWorld(lightFrustum, modelMatrixLocShadowMap);
		else mWorldRenderer.drawWorldOld(lightFrustum, modelMatrixLocShadowMap);

		glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapLowRes.fbo());
		glViewport(0, 0, mShadowMapLowRes.width(), mShadowMapLowRes.height());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!mOldWorldRenderer) mWorldRenderer.drawWorld(lightFrustum, modelMatrixLocShadowMap);
		else mWorldRenderer.drawWorldOld(lightFrustum, modelMatrixLocShadowMap);

		glDisable(GL_POLYGON_OFFSET_FILL);
		//glCullFace(GL_BACK);
		glDisable(GL_DEPTH_TEST);
		


		// Render Spotlight & light shafts stencil buffer

		glUseProgram(mStencilLightProgram.handle());

		glDisable(GL_CULL_FACE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);

		gl::setUniform(mStencilLightProgram, "uViewProjMatrix", projMatrix * viewMatrix);
		gl::setUniform(mStencilLightProgram, "uModelMatrix", spotlight.viewFrustumTransform());

		glBindFramebuffer(GL_FRAMEBUFFER, mSpotlightShadingFB.fbo());
		glViewport(0, 0, mSpotlightShadingFB.width(), mSpotlightShadingFB.height());
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		spotlight.renderViewFrustum();

		glBindFramebuffer(GL_FRAMEBUFFER, mLightShaftsFB.fbo());
		glViewport(0, 0, mLightShaftsFB.width(), mLightShaftsFB.height());
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);

		spotlight.renderViewFrustum();


		// Prepare for shading

		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass stencil test if not 0.
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);


		// Spotlight shading

		glUseProgram(mSpotlightShadingProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mSpotlightShadingFB.fbo());
		glViewport(0, 0, mSpotlightShadingFB.width(), mSpotlightShadingFB.height());

		stupidSetSpotlightUniform(mSpotlightShadingProgram, "uSpotlight", spotlight, viewMatrix, invViewMatrix);

		mPostProcessQuad.render();


		// Light shafts

		glUseProgram(mLightShaftsProgram.handle());
		glBindFramebuffer(GL_FRAMEBUFFER, mLightShaftsFB.fbo());
		glViewport(0, 0, mLightShaftsFB.width(), mLightShaftsFB.height());

		stupidSetSpotlightUniform(mLightShaftsProgram, "uSpotlight", spotlight, viewMatrix, invViewMatrix);

		mPostProcessQuad.render();


		glDisable(GL_STENCIL_TEST);
	}

	// Ambient Occlusion
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint aoTex = mSSAO.calculate(mGBuffer.texture(GBUFFER_LINEAR_DEPTH),
	                               mGBuffer.texture(GBUFFER_NORMAL),
	                               projMatrix, mCam.far());

	// Global shading
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	glUseProgram(mGlobalShadingProgram.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mFinalFB.fbo());
	glViewport(0, 0, mFinalFB.width(), mFinalFB.height());

	// Binding input textures
	gl::setUniform(mGlobalShadingProgram, "uLinearDepthTexture", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_LINEAR_DEPTH));

	gl::setUniform(mGlobalShadingProgram, "uNormalTexture", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_NORMAL));

	gl::setUniform(mGlobalShadingProgram, "uDiffuseTexture", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_DIFFUSE));
	
	gl::setUniform(mGlobalShadingProgram, "uMaterialTexture", 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_MATERIAL));
	
	gl::setUniform(mGlobalShadingProgram, "uAOTexture", 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, aoTex);

	gl::setUniform(mGlobalShadingProgram, "uSpotlightTexture", 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mSpotlightShadingFB.texture(0));

	gl::setUniform(mGlobalShadingProgram, "uLightShaftsTexture", 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, mLightShaftsFB.texture(0));

	gl::setUniform(mGlobalShadingProgram, "uLinearDepthTexture", 7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.texture(GBUFFER_LINEAR_DEPTH));

	gl::setUniform(mGlobalShadingProgram, "uInvProjMatrix", invProjMatrix);
	gl::setUniform(mGlobalShadingProgram, "uFarPlaneDist", mCam.far());
	gl::setUniform(mGlobalShadingProgram, "uAmbientLight", vec3{0.15f});
	gl::setUniform(mGlobalShadingProgram, "uOutputSelect", mOutputSelect);

	mPostProcessQuad.render();

	// Scaling to screen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	uint32_t scalingSrcTex = mFinalFB.texture(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWindow.width(), mWindow.height());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mScaler.scale(0, mWindow.drawableDimensions(), scalingSrcTex, mGBuffer.dimensionsFloat());

	// Rendering GUI
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	using gl::HorizontalAlign;
	using gl::VerticalAlign;
	FontRenderer& font = Assets::INSTANCE().mFontRenderer;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	float aspect = mWindow.drawableWidth() / mWindow.drawableHeight();
	vec2 fontWindowDimensions{100.0f * aspect, 100.0f};

	// Render frametime stats
	if (mCfg.printFrametimes) {
		char shortTermPerfBuffer[128];
		std::snprintf(shortTermPerfBuffer, 128, "Last %i frames: %s", mShortTermPerfStats.currentNumSamples(), mShortTermPerfStats.to_string());
		char longerTermPerfBuffer[128];
		std::snprintf(longerTermPerfBuffer, 128, "Last %i frames: %s", mLongerTermPerfStats.currentNumSamples(), mLongerTermPerfStats.to_string());
		char longestTermPerfBuffer[128];
		std::snprintf(longestTermPerfBuffer, 128, "Last %i frames: %s", mLongestTermPerfStats.currentNumSamples(), mLongestTermPerfStats.to_string());

		float fontSize = state.window.drawableHeight()/32.0f;
		float offset = fontSize*0.04f;
		float bottomOffset = state.window.drawableHeight()/25.0f;

		font.verticalAlign(gl::VerticalAlign::BOTTOM);
		font.horizontalAlign(gl::HorizontalAlign::LEFT);

		font.begin(state.window.drawableDimensions()/2.0f, state.window.drawableDimensions());
		font.write(vec2{offset, bottomOffset + fontSize*2.10f - offset}, fontSize, shortTermPerfBuffer);
		font.write(vec2{offset, bottomOffset + fontSize*1.05f - offset}, fontSize, longerTermPerfBuffer);
		font.write(vec2{offset, bottomOffset - offset}, fontSize, longestTermPerfBuffer);
		font.end(0, state.window.drawableDimensions(), sfz::vec4{0.0f, 0.0f, 0.0f, 1.0f});

		font.begin(state.window.drawableDimensions()/2.0f, state.window.drawableDimensions());
		font.write(vec2{0.0f, bottomOffset + fontSize*2.10f}, fontSize, shortTermPerfBuffer);
		font.write(vec2{0.0f, bottomOffset + fontSize*1.05f}, fontSize, longerTermPerfBuffer);
		font.write(vec2{0.0f, bottomOffset}, fontSize, longestTermPerfBuffer);
		font.end(0, state.window.drawableDimensions(), sfz::vec4{1.0f, 1.0f, 1.0f, 1.0f});
	}

	// Draw GUI
	font.horizontalAlign(HorizontalAlign::LEFT);
	font.verticalAlign(VerticalAlign::BOTTOM);

	// Drop shadow
	float xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.2f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.2f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel).c_str());
	font.end(0, mWindow.drawableDimensions(), vec4{0.0f, 0.0f, 0.0f, 1.0f});

	xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2{xPos, 0.5f}, 4.0f, "Voxel: ");
	font.write(vec2{xPos, 0.5f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel).c_str());
	font.end(0, mWindow.drawableDimensions(), vec4{1.0f, 1.0f, 1.0f, 1.0f});
}

void GameScreen::onQuit()
{

}

void GameScreen::onResize(vec2 dimensions, vec2 drawableDimensions)
{
	updateResolutions(drawableDimensions);
}

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void GameScreen::updatePrograms() noexcept
{
	mGBufferGenProgram = Program::fromFile((sfz::basePath() + "assets/shaders/gbuffer_gen.vert").c_str(),
	                                       (sfz::basePath() + "assets/shaders/gbuffer_gen.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindAttribLocation(shaderProgram, 1, "inNormal");
		glBindAttribLocation(shaderProgram, 2, "inUVCoord");
		glBindFragDataLocation(shaderProgram, 0, "outFragLinearDepth");
		glBindFragDataLocation(shaderProgram, 1, "outFragNormal");
		glBindFragDataLocation(shaderProgram, 2, "outFragDiffuse");
		glBindFragDataLocation(shaderProgram, 3, "outFragMaterial");
	});
	
	mShadowMapProgram = Program::fromFile((sfz::basePath() + "assets/shaders/shadow_map.vert").c_str(),
	                                      (sfz::basePath() + "assets/shaders/shadow_map.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindFragDataLocation(shaderProgram, 0, "outFragColor");
	});

	mStencilLightProgram = Program::fromFile((sfz::basePath() + "assets/shaders/stencil_shader.vert").c_str(),
	                                         (sfz::basePath() + "assets/shaders/stencil_shader.frag").c_str(),
		[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
	});

	mSpotlightShadingProgram = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/spotlight_shading.frag").c_str());
	
	mLightShaftsProgram = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/light_shafts.frag").c_str());
	
	mGlobalShadingProgram = Program::postProcessFromFile((sfz::basePath() + "assets/shaders/global_shading.frag").c_str());

	mScaler = gl::Scaler(gl::ScalingAlgorithm::BILINEAR);
}

void GameScreen::updateResolutions(vec2 drawableDim) noexcept
{
	vec2i internalRes{(int)std::round(mCfg.internalResScaling * drawableDim.x),
	                  (int)std::round(mCfg.internalResScaling * drawableDim.y)};

	if (internalRes == mGBuffer.dimensions()) return;
	mCam.setAspectRatio(drawableDim.x / drawableDim.y);

	vec2i ssaoRes{(int)std::round(mCfg.ssaoResScaling * internalRes.x),
	              (int)std::round(mCfg.ssaoResScaling * internalRes.y)};
	vec2i spotlightRes{(int)std::round(mCfg.spotlightResScaling * internalRes.x),
	                   (int)std::round(mCfg.spotlightResScaling * internalRes.y)};
	vec2i lightShaftsRes{(int)std::round(mCfg.lightShaftsResScaling * internalRes.x),
	                     (int)std::round(mCfg.lightShaftsResScaling * internalRes.y)};

	mGBuffer = gl::FramebufferBuilder{internalRes}
	          .addDepthBuffer(gl::FBDepthFormat::F32)
	          .addTexture(GBUFFER_LINEAR_DEPTH, gl::FBTextureFormat::R_F32, gl::FBTextureFiltering::NEAREST)
	          .addTexture(GBUFFER_NORMAL, gl::FBTextureFormat::RGB_F32, gl::FBTextureFiltering::NEAREST)
	          .addTexture(GBUFFER_DIFFUSE, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::NEAREST)
	          .addTexture(GBUFFER_MATERIAL, gl::FBTextureFormat::RGB_F32, gl::FBTextureFiltering::NEAREST)
	          .build();

	mSpotlightShadingFB = gl::FramebufferBuilder{spotlightRes}
	                     .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	                     .addStencilBuffer()
	                     .build();
	
	mLightShaftsFB =  gl::FramebufferBuilder{lightShaftsRes}
	                 .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	                 .addStencilBuffer()
	                 .build();
	
	mFinalFB = gl::FramebufferBuilder{internalRes}
	          .addTexture(0, gl::FBTextureFormat::RGB_U8, gl::FBTextureFiltering::LINEAR)
	          .build();

	mSSAO.dimensions(ssaoRes);
}

} // namespace vox

