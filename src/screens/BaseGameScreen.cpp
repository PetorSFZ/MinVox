#include "screens/BaseGameScreen.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

void checkGLErrorsMessage(const std::string& msg)
{
	if (gl::checkAllGLErrors()) std::cerr << msg << std::endl;
}

vec3f sphericalToCartesian(float r, float theta, float phi) noexcept
{
	using std::sinf;
	using std::cosf;
	return vec3f{r*sinf(theta)*sinf(phi), r*cosf(phi), r*cosf(theta)*sinf(phi)};
}

vec3f sphericalToCartesian(const vec3f& spherical) noexcept
{
	return sphericalToCartesian(spherical[0], spherical[1], spherical[2]);
}

void drawLight(int modelMatrixLoc, const vec3f& lightPos) noexcept
{
	static CubeObject cubeObj;
	static vec3f halfLightSize{2.0f, 2.0f, 2.0f};
	mat4f transform = sfz::scalingMatrix4<float>(4.0f);

	// Render sun
	sfz::translation(transform, lightPos - halfLightSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(Voxel{VOXEL_ORANGE}));
	cubeObj.render();
}

void drawSkyCube(int modelMatrixLoc, const Camera& cam) noexcept
{
	static SkyCubeObject skyCubeObj;
	static const vec3f halfSkyCubeSize{400.0f, 400.0f, 400.0f};
	mat4f transform = sfz::scalingMatrix4(800.0f, 800.0f, 800.0f);

	// Render skycube
	sfz::translation(transform, cam.mPos - halfSkyCubeSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(Voxel{VOXEL_VANILLA}));
	skyCubeObj.render();
}

void drawPlacementCube(int modelMatrixLoc, const vec3f& pos, Voxel voxel) noexcept
{
	static CubeObject cubeObj;
	gl::setUniform(modelMatrixLoc, sfz::translationMatrix<float>(pos - vec3f{0.025f, 0.025f, 0.025f})*sfz::scalingMatrix4<float>(1.05f));
	glBindTexture(GL_TEXTURE_2D, Assets::INSTANCE().cubeFaceIndividualTexture(voxel));
	cubeObj.render();
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

BaseGameScreen::BaseGameScreen(sdl::Window& window, const std::string& worldName)
:
	mCfg{getGlobalConfig()},

	mWorld{worldName, vec3f{-3.0f, 1.2f, 0.2f}, mCfg.mHorizontalRange, mCfg.mVerticalRange},
	mCam{vec3f{-3.0f, 2.5f, 0.2f}, vec3f{1.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f}, 75.0f,
	     (float)window.width()/(float)window.height(), 0.55f, 1000.0f},

	mWindow{window},

	mShadowMapShader{compileShadowMapShaderProgram()},
	mGBufferGenShader{compileGBufferGenShaderProgram()},
	mDirLightingShader{compileDirectionalLightingShaderProgram()},
	mGlobalLightingShader{compileGlobalLightingShaderProgram()},
	mOutputSelectShader{compileOutputSelectShaderProgram()},
	mShadowMap{mCfg.mShadowResolution, ShadowMapRes::BITS_16, mCfg.mShadowPCF, vec4f{0.f, 0.f, 0.f, 1.f}},
	mGBuffer{window.drawableWidth(), window.drawableHeight()},
	mDirLightFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mGlobalLightingFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mOutputSelectFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mSSAO{window.drawableWidth(), window.drawableHeight(), mCfg.mSSAONumSamples, mCfg.mSSAORadius, mCfg.mSSAOExp},
	mWorldRenderer{mWorld},

	mCurrentVoxel{VOXEL_AIR}
	//mSun{vec3f{0.0f, 0.0f, 0.0f}, vec3f{1.0f, 0.0f, 0.0f}, 3.0f, 80.0f, vec3f{0.2f, 0.25f, 0.8f}}
{
	updateResolutions((int)window.drawableWidth(), (int)window.drawableHeight());

	mProfiler = InGameProfiler{{"GBuffer Gen",
								"Directional Lights (+Shadow Maps)",
	                            "Global Lighting (+Shadows Map and SSAO)",
	                            "Text Rendering",
	                            "Output Select + Blitting",
	                            "Between Frames"}};

	//mLightPosSpherical = vec3f{60.0f, sfz::PI()*0.15f, sfz::PI()*0.35f}; // [0] = r, [1] = theta, [2] = phi
	//mLightTarget = vec3f{16.0f, 0.0f, 16.0f};

	// First corridor
	vec3f f1Color{0.0f, 0.0f, 1.0f};
	mLights.emplace_back(vec3f{-21.430313, 5.780775, 5.168257}, vec3f{0.499439, -0.200375, 0.842858}, 0.5f, 20.0f, f1Color);
	mLights.emplace_back(vec3f{-21.720879, 1.155828, 15.699636}, vec3f{-0.563084, 0.218246, -0.797059}, 0.5f, 20.0f, f1Color);

	// Staircase
	mLights.emplace_back(vec3f{-33.711731, 13.120087, 32.218548}, vec3f{0.038979, -0.521176, -0.852557}, 0.5f, 40.0f, vec3f{0.8f, 0.2f, 0.8f});

	// Second corridor
	vec3f f2Color{0.0f, 1.0f, 0.0f};
	mLights.emplace_back(vec3f{-23.068808, 8.956177, 33.155720}, vec3f{-0.092388, -0.226080, -0.969712}, 0.5f, 20.0f, f2Color);
	mLights.emplace_back(vec3f{-20.271776, 2.191609, 26.143528}, vec3f{-0.271371, 0.962427, 0.009065}, 0.5f, 20.0f, f2Color);

	// Balcony
	mLights.emplace_back(vec3f{-17.184530, 10.616333, 26.045494}, vec3f{0.932476, -0.361071, -0.010368}, 0.5f, 100.0f, vec3f{0.4f, 0.5f, 0.9f});

	// Semi-global
	mLights.emplace_back(vec3f{46.868477, 32.830544, 18.390802}, vec3f{-0.776988, -0.629503, 0.004005}, 35.0f, 120.0f, vec3f{0.2f, 0.25f, 0.8f});


	mProfiler.startProfiling();
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
				updateResolutions(event.window.data1, event.window.data2);
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
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

			case 'l':{
				std::random_device rd;
				std::mt19937_64 gen{rd()};
				std::uniform_real_distribution<float> distr{0.0f, 1.0f};
				mLights.emplace_back(mCam.mPos, mCam.mDir, 0.5f, 40.0f, vec3f{distr(gen), distr(gen), distr(gen)});
				std::cout << "Light: Pos: " << mLights.back().mCam.mPos << ", Dir: " << mLights.back().mCam.mDir << ", Color: " << mLights.back().mColor << std::endl;
				}break;
			case 'o':
				if (mLights.size() > 1) mLights.pop_back();
				break;

			/*case 'l':
				std::random_device rd;
				std::mt19937_64 gen{rd()};
				std::uniform_real_distribution<float> distr{0.0f, 1.0f};
				mSun.mColor = vec3f{distr(gen), distr(gen), distr(gen)};
				std::cout << "New random light color: " << mSun.mColor << std::endl;
				break;*/
			}
			break;
		}
		
	}

	updateSpecific(events, ctrl, delta);

	/*if (currentLightAxis != -1) {
		mLightPosSpherical[currentLightAxis] += delta * lightCurrentSpeed;
		mLightPosSpherical[currentLightAxis] = std::fmod(mLightPosSpherical[currentLightAxis],
		                                                (sfz::PI()*2.0f));
	}*/

	mCam.updateMatrices();
	mCam.updatePlanes();
	mWorld.update(mCam.mPos);
}

void BaseGameScreen::render(float delta)
{
	mProfiler.endProfiling(5);

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

	mProfiler.startProfiling();

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
	gl::setUniform(mGBufferGenShader, "uEmissive", vec3f{0.15f, 0.15f, 0.2f});
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3f{0.0f, 0.0f, 0.0f});
	drawSkyCube(modelMatrixLocGBufferGen, mCam);

	gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
	gl::setUniform(mGBufferGenShader, "uEmissive", vec3f{0.0f, 0.0f, 0.0f});
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3f{1.0, 0.50, 0.25});
	if (!mOldWorldRenderer) mWorldRenderer.drawWorld(mCam, modelMatrixLocGBufferGen);
	else mWorldRenderer.drawWorldOld(mCam, modelMatrixLocGBufferGen);

	/*gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
	gl::setUniform(mGBufferGenShader, "uEmissive", mSun.mColor*0.5f);
	gl::setUniform(mGBufferGenShader, "uMaterial", vec3f{0.0f, 0.0f, 0.0f});
	drawLight(modelMatrixLocGBufferGen, mSun.mCam.mPos);*/
	
	if (mCurrentVoxel.mType != VOXEL_AIR && mCurrentVoxel.mType != VOXEL_LIGHT) {
		gl::setUniform(mGBufferGenShader, "uHasEmissiveTexture", 0);
		gl::setUniform(mGBufferGenShader, "uEmissive", vec3f{0.15f, 0.15f, 0.15f});
		gl::setUniform(mGBufferGenShader, "uMaterial", vec3f{1.0, 0.50, 0.25});
		drawPlacementCube(modelMatrixLocGBufferGen, mCurrentVoxelPos, mCurrentVoxel);
	}

	mProfiler.endProfiling(0);

	checkGLErrorsMessage("^^^ Errors caused by: render() GBuffer.");

	// Directional Lights (+Shadow Maps)
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	mProfiler.startProfiling();

	// Clear directional lighting texture
	glUseProgram(mDirLightingShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.mFBO);
	glViewport(0, 0, mDirLightFramebuffer.mWidth, mDirLightFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4f inverseViewMatrix = inverse(mCam.mViewMatrix);

	for (auto& light : mLights) {
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

		// Render light
		glUseProgram(mDirLightingShader);
		glBindFramebuffer(GL_FRAMEBUFFER, mDirLightFramebuffer.mFBO);
		glViewport(0, 0, mDirLightFramebuffer.mWidth, mDirLightFramebuffer.mHeight);

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

		mFullscreenQuad.render();
	
		glUseProgram(0);
	}

	mProfiler.endProfiling(1);

	checkGLErrorsMessage("^^^ Errors caused by: render() directional lights.");

	// Global Lighting + SSAO + Shadow Map
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	mProfiler.startProfiling();

	GLuint aoTex = mSSAO.calculate(mGBuffer.mPositionTexture, mGBuffer.mNormalTexture,
	                                      mCam.mProjMatrix);

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

	gl::setUniform(mGlobalLightingShader, "uAmbientLight", vec3f{0.2f, 0.2f, 0.2f});

	mFullscreenQuad.render();
	
	glUseProgram(0);

	mProfiler.endProfiling(2);

	checkGLErrorsMessage("^^^ Errors caused by: render() Global Lighting + SSAO + Shadow Map");

	// Rendering some text
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	mProfiler.startProfiling();

	using gl::HorizontalAlign;
	using gl::VerticalAlign;

	float aspect = (float)mGlobalLightingFramebuffer.mWidth / (float)mGlobalLightingFramebuffer.mHeight;
	vec2f fontWindowDimensions{100.0f * aspect, 100.0f};
	vec2f lightingViewport{(float)mGlobalLightingFramebuffer.mWidth, (float)mGlobalLightingFramebuffer.mHeight};

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
		font.horizontalAlign(HorizontalAlign::LEFT);
		font.verticalAlign(VerticalAlign::TOP);

		// Drop shadow
		float xPos = 1.15f;
		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		xPos = font.write(vec2f{xPos, 99.85f}, fontSize, "FPS: ");
		xPos = font.write(vec2f{xPos, 99.85f}, fontSize, std::to_string(fps));
		xPos = font.write(vec2f{xPos, 99.85f}, fontSize, ", Mean: ");
		xPos = font.write(vec2f{xPos, 99.85f}, fontSize, std::to_string(mFPSMean));
		for (size_t i = 0; i < mProfiler.size(); ++i) {
			font.write(vec2f{1.0f, 99.85f - fontSize*(i+1)}, fontSize,
			                    mProfiler.completeString(i));
		}
		font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport,
						  vec4f{0.0f, 0.0f, 0.0f, 1.0f});

		xPos = 1.0f;
		font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
		xPos = font.write(vec2f{xPos, 100.0f}, fontSize, "FPS: ");
		xPos = font.write(vec2f{xPos, 100.0f}, fontSize, std::to_string(fps));
		xPos = font.write(vec2f{xPos, 100.0f}, fontSize, ", Mean: ");
		xPos = font.write(vec2f{xPos, 100.0f}, fontSize, std::to_string(mFPSMean));
		for (size_t i = 0; i < mProfiler.size(); ++i) {
			font.write(vec2f{1.0f, 100.0f - fontSize*(i+1)}, fontSize,
			                    mProfiler.completeString(i));
		}
		font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport,
						  vec4f{1.0f, 0.0f, 1.0f, 1.0f});
	}

	// Draw GUI
	font.horizontalAlign(HorizontalAlign::LEFT);
	font.verticalAlign(VerticalAlign::BOTTOM);

	// Drop shadow
	float xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2f{xPos, 0.2f}, 4.0f, "Voxel: ");
	font.write(vec2f{xPos, 0.2f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel));
	font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport, vec4f{0.0f, 0.0f, 0.0f, 1.0f});

	xPos = 1.15f;
	font.begin(fontWindowDimensions/2.0f, fontWindowDimensions);
	xPos = font.write(vec2f{xPos, 0.5f}, 4.0f, "Voxel: ");
	font.write(vec2f{xPos, 0.5f}, 4.0f, Assets::INSTANCE().cubeFaceName(mCurrentVoxel));
	font.end(mGlobalLightingFramebuffer.mFBO, lightingViewport, vec4f{1.0f, 1.0f, 1.0f, 1.0f});


	mProfiler.endProfiling(3);

	checkGLErrorsMessage("^^^ Errors caused by: render() text rendering.");

	// Output select
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	mProfiler.startProfiling();

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

	mProfiler.endProfiling(4);

	mProfiler.startProfiling();

	checkGLErrorsMessage("^^^ Errors caused by: render() blitting.");
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

void BaseGameScreen::quitApplication() noexcept
{
	mQuit = true;
}

void BaseGameScreen::changeScreen(IScreen* newScreen) noexcept
{
	mNewScreenPtr = newScreen;
}

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void BaseGameScreen::updateResolutions(int width, int height) noexcept
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

void BaseGameScreen::reloadFramebuffers(int width, int height) noexcept
{
	mGBuffer = GBuffer{width, height};
	mDirLightFramebuffer = PostProcessFramebuffer{width, height};
	mGlobalLightingFramebuffer = PostProcessFramebuffer{width, height};
	mOutputSelectFramebuffer = PostProcessFramebuffer{width, height};
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>