#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include <string>
#include <memory>
#include <random>

#include "sfz/GL.hpp"

#include "IO.hpp"
#include "Model.hpp"
#include "Rendering.hpp"
#include "screens/IScreen.hpp"
#include "GlobalConfig.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3f;
using sfz::vec4f;
using sfz::mat4f;

class BaseGameScreen : public IScreen {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	BaseGameScreen() = delete;
	//BaseGameScreen(const BaseGameScreen&) = delete;
	BaseGameScreen& operator= (const BaseGameScreen&) = delete;

	BaseGameScreen(sdl::Window& window, const std::string& worldName);
	//BaseGameScreen(const BaseGameScreen& baseGameScreen);
	virtual ~BaseGameScreen();

	// Overriden methods from IScreen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const std::vector<SDL_Event>& events,
	            const sdl::GameController& ctrl, float delta) override final;
	std::unique_ptr<IScreen> changeScreen() override final;
	bool quit() override final;
	void render(float delta) override final;

protected:
	// Protected methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	virtual void updateSpecific(const std::vector<SDL_Event>& events,
	                            const sdl::GameController& ctrl, float delta) = 0;

	void quitApplication() noexcept;

	void changeScreen(IScreen* newScreen) noexcept;

	// Protected members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GlobalConfig& mCfg;

	World mWorld;
	Camera mCam;

	sdl::Window& mWindow;
	Assets mAssets;

	GLuint mShadowMapShader, mGBufferGenShader, mLightingShader, mOutputSelectShader;
	ShadowMap mShadowMap;
	GBuffer mGBuffer;
	PostProcessFramebuffer mLightingFramebuffer, mOutputSelectFramebuffer;
	SSAO mSSAO;
	sfz::FontRenderer mFontRenderer;
	FullscreenQuadObject mFullscreenQuad;
	WorldRenderer mWorldRenderer;
	
	bool mOldWorldRenderer = false;
	int mRenderMode = 1;

	Camera mSunCam;
	vec3f mLightPosSpherical, mLightTarget, mLightColor;

	int currentLightAxis = 1;
	float lightCurrentSpeed = 1.0f;
	float lightNormalSpeed = 0.5f;
	float lightMaxSpeed = sfz::PI();

	float mFPSMean = 0.0f;
	long mFPSSamples = 0;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void reloadFramebuffers(int width, int height) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	IScreen* mNewScreenPtr = nullptr;
	bool mQuit = false;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif