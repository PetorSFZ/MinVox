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
#include <sfz/util/FrametimeStats.hpp>

#include "IO.hpp"
#include "Model.hpp"
#include "Rendering.hpp"
#include "GlobalConfig.hpp"

namespace vox {

using gl::Framebuffer;
using gl::Program;
using gl::Spotlight;
using gl::ViewFrustum;

using sfz::vec2;
using sfz::vec3;
using sfz::vec4;
using sfz::vec2i;
using sfz::vec3i;
using sfz::vec4i;
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

	void updatePrograms() noexcept;
	void updateResolutions(vec2 drawableDim) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GlobalConfig& mCfg;
	sdl::Window& mWindow;
	World mWorld;

	gl::PostProcessQuad mPostProcessQuad;
	Program mGBufferGenProgram, mShadowMapProgram, mStencilLightProgram, mSpotlightShadingProgram,
	        mLightShaftsProgram, mGlobalShadingProgram;
	
	gl::SSAO mSSAO;
	Framebuffer mGBuffer, mSpotlightShadingFB, mLightShaftsFB;
	Framebuffer mShadowMapHighRes, mShadowMapLowRes;
	
	ViewFrustum mCam;
	vector<Spotlight> mSpotlights;

	WorldRenderer mWorldRenderer;
	bool mOldWorldRenderer = false;
	int mOutputSelect = 1;

	vec3 mCurrentVoxelPos; // TODO: Move this to CreationGameScreen
	Voxel mCurrentVoxel; // TODO: Move this to CreationGameScreen

	sfz::FrametimeStats mShortTermPerfStats, mLongerTermPerfStats, mLongestTermPerfStats;
};

} // namespace vox

#endif