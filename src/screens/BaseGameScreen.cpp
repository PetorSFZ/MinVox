#include "screens/BaseGameScreen.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

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

void drawLight(const Assets& assets, int modelMatrixLoc, const vec3f& lightPos) noexcept
{
	static CubeObject cubeObj;
	mat4f transform = sfz::identityMatrix4<float>();

	// Render sun
	sfz::translation(transform, lightPos);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, assets.YELLOW.mHandle);
	cubeObj.render();
}

void drawSkyCube(const Assets& assets, int modelMatrixLoc, const Camera& cam) noexcept
{
	static SkyCubeObject skyCubeObj;
	static const vec3f halfSkyCubeSize{400.0f, 400.0f, 400.0f};
	mat4f transform = sfz::scalingMatrix4(800.0f, 800.0f, 800.0f);

	// Render skycube
	sfz::translation(transform, cam.mPos - halfSkyCubeSize);
	gl::setUniform(modelMatrixLoc, transform);
	glBindTexture(GL_TEXTURE_2D, assets.VANILLA.mHandle);
	skyCubeObj.render();
}

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

BaseGameScreen::BaseGameScreen(sdl::Window& window, const std::string& worldName)
:
	mCfg{getGlobalConfig()},

	mWorld{worldName, vec3f{-3.0f, 1.2f, 0.2f}, mCfg.mHorizontalRange, mCfg.mVerticalRange},
	mCam{vec3f{-3.0f, 1.2f, 0.2f}, vec3f{1.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f}, 75.0f,
	     (float)window.width()/(float)window.height(), 0.5f, 1000.0f},

	mWindow{window},
	mAssets{},

	mShadowMapShader{compileShadowMapShaderProgram()},
	mGBufferGenShader{compileGBufferGenShaderProgram()},
	mLightingShader{compileLightingShaderProgram()},
	mOutputSelectShader{compileOutputSelectShaderProgram()},
	mShadowMap{4096, ShadowMapRes::BITS_32, true, vec4f{1.f, 1.f, 1.f, 1.f}},
	mGBuffer{window.drawableWidth(), window.drawableHeight()},
	mLightingFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mOutputSelectFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mSSAO{window.drawableWidth(), window.drawableHeight(), mCfg.mSSAONumSamples, mCfg.mSSAORadius, mCfg.mSSAOExp},
	mFontRenderer{assetsPath() + "fonts/SourceCodePro-Regular.ttf", 64.0f},
	mFontFramebuffer{window.drawableWidth(), window.drawableHeight()},
	mWorldRenderer{mWorld, mAssets},

	mSunCam{vec3f{0.0f, 0.0f, 0.0f}, vec3f{1.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f},
	        65.0f, 1.0f, 3.0f, 120.0f}
{
	mLightPosSpherical = vec3f{60.0f, sfz::PI()*0.15f, sfz::PI()*0.35f}; // [0] = r, [1] = theta, [2] = phi
	mLightTarget = vec3f{16.0f, 0.0f, 16.0f};
	mLightColor = vec3f{1.0f, 0.85f, 0.75f};
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
				mCam.mAspectRatio = w/h;
				reloadFramebuffers(event.window.data1, event.window.data2);
				mSSAO.textureSize(event.window.data1, event.window.data2);
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
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

			case 'p':
				std::random_device rd;
				std::mt19937_64 gen{rd()};
				std::uniform_real_distribution<float> distr{0.0f, 1.0f};
				mLightColor = vec3f{distr(gen), distr(gen), distr(gen)};
				std::cout << "New random light color: " << mLightColor << std::endl;
				break;
			}
			break;
		}
		
	}

	updateSpecific(events, ctrl, delta);

	if (currentLightAxis != -1) {
		mLightPosSpherical[currentLightAxis] += delta * lightCurrentSpeed;
		mLightPosSpherical[currentLightAxis] = std::fmod(mLightPosSpherical[currentLightAxis],
		                                                (sfz::PI()*2.0f));
	}

	mCam.updateMatrices();
	mCam.updatePlanes();
	mWorld.update(mCam.mPos);
}

void BaseGameScreen::render(float)
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

	glUseProgram(mShadowMapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap.mFBO);
	glViewport(0, 0, mShadowMap.mResolution, mShadowMap.mResolution);

	// Light position and matrices
	mSunCam.mPos = sphericalToCartesian(mLightPosSpherical);
	mSunCam.mDir = (mLightTarget - mSunCam.mPos).normalize();
	mSunCam.updateMatrices();
	mSunCam.updatePlanes();
	
	gl::setUniform(mShadowMapShader, "uViewMatrix", mSunCam.mViewMatrix);
	gl::setUniform(mShadowMapShader, "uProjectionMatrix", mSunCam.mProjMatrix);

	// Clear shadow map
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Fix surface acne
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(5.0f, 25.0f);
	//glCullFace(GL_FRONT);

	// Draw shadow casters
	int modelMatrixLocShadowMap = glGetUniformLocation(mShadowMapShader, "uModelMatrix");
	if (!mOldWorldRenderer) mWorldRenderer.drawWorld(mSunCam, modelMatrixLocShadowMap);
	else mWorldRenderer.drawWorldOld(mSunCam, modelMatrixLocShadowMap);

	// Cleanup
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glCullFace(GL_BACK);

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
	drawSkyCube(mAssets, modelMatrixLocGBufferGen, mCam);
	if (!mOldWorldRenderer) mWorldRenderer.drawWorld(mCam, modelMatrixLocGBufferGen);
	else mWorldRenderer.drawWorldOld(mCam, modelMatrixLocGBufferGen);
	drawLight(mAssets, modelMatrixLocGBufferGen, mSunCam.mPos);

	// Lighting
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint occlusionTex = mSSAO.calculate(mGBuffer.mPositionTexture, mGBuffer.mNormalTexture,
	                                      mCam.mProjMatrix);

	glUseProgram(mLightingShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mLightingFramebuffer.mFBO);
	glViewport(0, 0, mLightingFramebuffer.mWidth, mLightingFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mDiffuseTexture);
	gl::setUniform(mLightingShader, "uDiffuseTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mPositionTexture);
	gl::setUniform(mLightingShader, "uPositionTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBuffer.mNormalTexture);
	gl::setUniform(mLightingShader, "uNormalTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, occlusionTex);
	gl::setUniform(mLightingShader, "uOcclusionTexture", 3);

	// Shadow map uniform
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mShadowMap.mDepthTexture);
	gl::setUniform(mLightingShader, "uShadowMap", 4);

	// Set view matrix uniform
	gl::setUniform(mLightingShader, "uViewMatrix", mCam.mViewMatrix);

	// Calculate and set lightMatrix
	mat4f lightMatrix = sfz::translationMatrix(0.5f, 0.5f, 0.5f)
	                  * sfz::scalingMatrix4(0.5f)
	                  * mSunCam.mProjMatrix
	                  * mSunCam.mViewMatrix
	                  * inverse(mCam.mViewMatrix);
	gl::setUniform(mLightingShader, "uLightMatrix", lightMatrix);

	// Set light position uniform
	gl::setUniform(mLightingShader, "uLightPos", mSunCam.mPos);
	gl::setUniform(mLightingShader, "uLightColor", mLightColor);

	mFullscreenQuad.render();
	
	glUseProgram(0);

	// Rendering some text
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glBindFramebuffer(GL_FRAMEBUFFER, mFontFramebuffer.mFBO);
	glViewport(0, 0, mFontFramebuffer.mWidth, mFontFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mFontRenderer.begin(vec2f{50.0f, 50.0f}, vec2f{100.0f, 100.0f});

	mFontRenderer.write(vec2f{10.0f, 70.0f}, 3.0f, "Hello World?åäö");

	mFontRenderer.writeBitmapFont(vec2f{25.0f, 25.0f}, vec2f{50.0f, 50.0f});

	mFontRenderer.end(mFontFramebuffer.mFBO, vec2f{static_cast<float>(mFontFramebuffer.mWidth),
	                                               static_cast<float>(mFontFramebuffer.mHeight)},
					  vec4f{1.0f, 0.0f, 1.0f, 1.0f});

	// Output select
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mOutputSelectShader);
	glBindFramebuffer(GL_FRAMEBUFFER, mOutputSelectFramebuffer.mFBO);
	glViewport(0, 0, mOutputSelectFramebuffer.mWidth, mOutputSelectFramebuffer.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mLightingFramebuffer.mTexture);
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
	glBindTexture(GL_TEXTURE_2D, occlusionTex);
	gl::setUniform(mOutputSelectShader, "uOcclusionTexture", 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, mFontFramebuffer.mTexture);
	gl::setUniform(mOutputSelectShader, "uFontTexture", 5);

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
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mOutputSelectFramebuffer.mFBO);
	glBlitFramebuffer(0, 0, mOutputSelectFramebuffer.mWidth, mOutputSelectFramebuffer.mHeight,
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

void BaseGameScreen::reloadFramebuffers(int width, int height) noexcept
{
	mGBuffer = GBuffer{width, height};
	mLightingFramebuffer = PostProcessFramebuffer{width, height};
	mOutputSelectFramebuffer = PostProcessFramebuffer{width, height};

	mFontFramebuffer = PostProcessFramebuffer{width, height};
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>