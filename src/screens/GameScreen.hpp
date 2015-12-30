#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include <string>
#include <memory>
#include <random>
#include <vector>

#include <sfz/geometry/ViewFrustum.hpp>
#include <sfz/GL.hpp>
#include <sfz/Screens.hpp>


#include "IO.hpp"
#include "Model.hpp"
#include "Rendering.hpp"
#include "GlobalConfig.hpp"

namespace vox {

using gl::Framebuffer;
using gl::Spotlight;
using gl::ViewFrustum;

using sfz::vec3;
using sfz::vec4;
using sfz::mat4;

using sfz::UpdateOp;
using sfz::UpdateState;
using std::unordered_map;
using std::vector;

class GameScreen final : public sfz::BaseScreen {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GameScreen() = delete;
	//GameScreen(const GameScreen&) = delete;
	GameScreen& operator= (const GameScreen&) = delete;

	GameScreen(sdl::Window& window, const std::string& worldName);
	//GameScreen(const GameScreen& baseGameScreen);
	//virtual ~GameScreen();

	// Overriden methods from sfz::BaseScreen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	virtual UpdateOp update(UpdateState& state) override final;
	virtual void render(UpdateState& state) override final;
	virtual void onQuit() override final;
	virtual void onResize(vec2 dimensions, vec2 drawableDimensions) override final;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void updateResolutions(int width, int height) noexcept;
	void reloadFramebuffers(int width, int height) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GlobalConfig& mCfg;

	World mWorld;
	ViewFrustum mCam;

	sdl::Window& mWindow;

	gl::Program mShadowMapShader, mGBufferGenShader, mDirLightingStencilShader, mDirLightingShader,
	            mGlobalLightingShader, mOutputSelectShader;
	Framebuffer mShadowMap;
	
	Framebuffer mGBuffer;
	Framebuffer mDirLightFramebuffer;
	Framebuffer mGlobalLightingFramebuffer, mOutputSelectFramebuffer;
	
	SSAO mSSAO;
	gl::PostProcessQuad mFullscreenQuad;
	WorldRenderer mWorldRenderer;
	
	bool mOldWorldRenderer = false;
	int mRenderMode = 1;

	vec3 mCurrentVoxelPos; // TODO: Move this to CreationGameScreen
	Voxel mCurrentVoxel; // TODO: Move this to CreationGameScreen

	/*DirectionalLight mSun;
	vec3 mLightPosSpherical, mLightTarget;

	int currentLightAxis = 1;
	float lightCurrentSpeed = 1.0f;
	float lightNormalSpeed = 0.5f;
	float lightMaxSpeed = sfz::PI();*/
	
	vector<Spotlight> mSpotlights;

	float mLightShaftExposure = 0.4f;

	float mFPSMean = 0.0f;
	long mFPSSamples = 0;
};

} // namespace vox

#endif